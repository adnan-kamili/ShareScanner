#include "scan.h"


/* Start of code from Samba */
int name_mangle( char *In, char *Out) {
	int   i,c;
	char  buf[20], *p = Out;
	
	(void)memset( buf, 0, 20 ); /* Safely copy the input string, In, into buf[]. */
	buf[0] = '*'; 
	p[0] = 32; /* Place the length of the first field into the output buffer. */
	p++;
					/* Now convert the name to the rfc1001/1002 format. */
	for( i = 0; i < 16; i++ ) {
		c = toupper( buf[i] );
		p[i*2]     = ( (c >> 4) & 0x000F ) + 'A';
		p[(i*2)+1] = (c & 0x000F) + 'A';
	}
	p += 32;
	p[0] = '\0';

    return( strlen(Out) );
}; /* name_mangle ,end of code from Samba */
    // Send Query
void send_query(int sock, struct in_addr dest_addr, uint32_t rtt_base){
	struct nbname_request request;
	struct sockaddr_in dest_sockaddr;
	int status;
	struct timeval tv;
	char errmsg[80];

    bzero((void*)&dest_sockaddr, sizeof(dest_sockaddr));
    dest_sockaddr.sin_family = AF_INET;
    dest_sockaddr.sin_port = htons(NB_DGRAM);
    dest_sockaddr.sin_addr = dest_addr;
 
	gettimeofday(&tv, NULL);
		
	request.transaction_id = htons((tv.tv_sec-rtt_base)*1000+tv.tv_usec/1000);
	request.flags = htons(FL_BROADCAST);
    request.question_count = htons(1);
    request.answer_count = 0;
    request.name_service_count = 0;
    request.additional_record_count = 0;
    name_mangle("*", request.question_name);
    request.question_type = htons(0x21);
    request.question_class = htons(0x01); 
	
	status = sendto(sock, (char*)&request, sizeof(request), 0, (struct sockaddr *)&dest_sockaddr, sizeof(dest_sockaddr));
	if(status==-1) {
	        snprintf(errmsg, 80, "%s\tSendto failed", inet_ntoa(dest_addr));
	        perror(errmsg);
        };
};


struct nb_host_info* parse_response(char* buff, int buffsize) {
	struct nb_host_info* hostinfo = NULL;
	int name_table_size;
	int offset = 0;
	
	if((hostinfo = malloc(sizeof(struct nb_host_info)))==NULL) return NULL;
    hostinfo->names = NULL;
	
	/* Parsing received packet, Start with header */
	if( offset+ 11* sizeof(uint16_t) >= buffsize) goto broken_packet;  	// Check if there is room for next field in buffer
        offset+= 11* sizeof(uint16_t) + 34;
	
	if( offset+sizeof(hostinfo->number_of_names) >= buffsize) goto broken_packet;
	hostinfo->number_of_names = *(typeof(hostinfo->number_of_names)*)(buff+offset);
        offset+=sizeof(hostinfo->number_of_names);
        
    /* checking name table field */	
	name_table_size = (hostinfo->number_of_names) * (sizeof(struct nbname));
	if( offset+name_table_size >= buffsize) goto broken_packet;
	
	if((hostinfo->names = malloc(name_table_size))==NULL) return NULL;
	memcpy(hostinfo->names, buff + offset, name_table_size);
	
	offset+=name_table_size;

	/* Done with name table - it is okay */ 

	if( offset+sizeof(hostinfo->adapter_address) >= buffsize) goto broken_packet;	
	memcpy(hostinfo->adapter_address, 
	       (buff+offset), 
	       sizeof(hostinfo->adapter_address));
	offset+=sizeof(hostinfo->adapter_address);

	return hostinfo;
	
	broken_packet: 
		hostinfo->is_broken = offset;
		return hostinfo;	
}

char* scan(char *target_string){
	int timeout=1000, sock, addr_size, size, flag = 1;
	struct sockaddr_in src_sockaddr, dest_sockaddr;
	struct  in_addr *target_address;
	struct timeval select_timeout, start, end;
	float rtt = 0.0, t1 = 0.0, t2 = 0.0;
	fd_set *fdsr, *fdsw;
	uint32_t rtt_base; 
	void *buff;
	struct nb_host_info* hostinfo;
	
	if(inet_addr(target_string) == INADDR_NONE)
		printf("Error: %s is not an IP address.\n", target_string), perror("Usage: ./scan [IpAddress]\n");
	target_address = malloc(sizeof(struct  in_addr)); 
	if(!target_address) 
		perror("Malloc failed"), exit(1);
	target_address->s_addr = inet_addr(target_string);
	if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		perror("Failed to create socket"), exit(1);
	
	// preparing select() arguments	
	fdsr=malloc(sizeof(fd_set));
	if(!fdsr) 
		perror("Malloc failed"), exit(1);
	FD_ZERO(fdsr);
	FD_SET(sock, fdsr);
	fdsw=malloc(sizeof(fd_set));
	if(!fdsw) 
		perror("Malloc failed"), exit(1);
	FD_ZERO(fdsw);
	FD_SET(sock, fdsw);
	select_timeout.tv_sec = 60, select_timeout.tv_usec = 0; 
	// end of select() arguments

	bzero((void*)&src_sockaddr, sizeof(src_sockaddr));
	src_sockaddr.sin_family = AF_INET;
	if (bind(sock, (struct sockaddr *)&src_sockaddr, sizeof(src_sockaddr)) == -1)
		perror("Failed to bind"), exit(1);
		
	buff=malloc(BUFFSIZE); 
	if(!buff) 
		perror("Malloc failed"),exit(1);
	
	gettimeofday(&start, NULL); /* Get current time */
	rtt_base = start.tv_sec; 
	
	while ( (select(sock+1, fdsr, fdsw, NULL, &select_timeout)) > 0) {
		if(FD_ISSET(sock, fdsr)) {
			addr_size = sizeof(struct sockaddr_in);
			if ( (size = recvfrom(sock, buff, BUFFSIZE, 0,(struct sockaddr*)&dest_sockaddr, (socklen_t *)&addr_size)) <= 0 ) 
				continue;
			if(!(hostinfo = (struct nb_host_info*)parse_response(buff, size))) 
				continue;
				
			gettimeofday(&end,NULL);
			t2 = (end.tv_usec/1000000.0); // in seconds
			
			rtt = (t2- t1)*1000;  // rtt in ms
			/*****************************************************************************/
			int i,unique, first_name=1;
            unsigned char service; // 16th byte of NetBIOS name 
            char comp_name[16];
            static char temp[200];
            strncpy(comp_name,"<unknown>",15);
            if(hostinfo) 
            {
            for(i=0; i< hostinfo->number_of_names; i++) {
            service = hostinfo->names[i].ascii_name[15];
            unique = ! (hostinfo->names[i].rr_flags & 0x0080);
            if(service == 0  && unique && first_name) {
	    	strncpy(comp_name, hostinfo->names[i].ascii_name, 15);
		    comp_name[15] = 0;
		    first_name = 0;
            }
           }
           sprintf(temp,"%s<*>%s<*>%02x:%02x:%02x:%02x:%02x:%02x<*>%f ms<*>",inet_ntoa(dest_sockaddr.sin_addr),comp_name,
	       hostinfo->adapter_address[0], hostinfo->adapter_address[1],
	       hostinfo->adapter_address[2], hostinfo->adapter_address[3],
	       hostinfo->adapter_address[4], hostinfo->adapter_address[5],
	       rtt);
	      // puts(temp);
	       return temp;
           } 
           else 
		      printf("\t");
           
			/*****************************************************************************/
			puts(temp);
			
			free(hostinfo);	
		}
		FD_ZERO(fdsr);
		FD_SET(sock, fdsr);		
		if(flag) {
			send_query(sock, *target_address, rtt_base);	
			flag =0;	
			gettimeofday(&start,NULL);   // updating start
			t1 = (start.tv_usec/1000000.0);
		}else { // No more queries to send
			FD_ZERO(fdsw);// timeout is in milliseconds
			select_timeout.tv_sec = timeout / 1000;
			select_timeout.tv_usec = (timeout % 1000) * 1000; // Microseconds
			continue;
		} 
	} // end while() loop
	return "NOTFOUND";
}
