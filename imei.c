

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>

typedef  unsigned char      boolean;     /* Boolean value type. */
#define TRUE   1   /* Boolean true value. */
#define FALSE  0   /* Boolean false value. */

typedef  unsigned char     byte;         /* Unsigned 8  bit value type. */

#include "diag_lsm.h"




#define DIAG_BUF_LEN    139
unsigned char diag_data[DIAG_BUF_LEN];
int data_primary = 100;

void init_nv()
{
	int i;
        
	diag_data[0] = 0x4B;
	diag_data[1] = 0x30;
	diag_data[2] = 0x01;  //read 0x02 =write
	diag_data[3] = 0;
    
    diag_data[4] = 0x26;
    diag_data[5] = 0x02;
    diag_data[6] = 1;    //slot
    diag_data[7] = 0;
    
	for (i = 8; i < DIAG_BUF_LEN; i++)
		diag_data[i] = 0;
}


int process_diag_data(unsigned char *ptr, int len, void *context_data)
{
	int i;
    
	DIAG_LOGE("\n From primary proc %d\n", *(int *)context_data);
	for (i = 0; i < len; i++) {
		DIAG_LOGE("%02x\t", ptr[i]);
	}
	DIAG_LOGE("\n\n");
	return 0;
}



boolean init_diag() {
	boolean bInit_Success = FALSE;

    
	bInit_Success = Diag_LSM_Init(NULL);
	if (bInit_Success) {
        setvbuf(stdout , NULL , _IOLBF , 4096);
        diag_register_callback(&process_diag_data, &data_primary);
        diag_switch_logging(CALLBACK_MODE, NULL);
    }

    return bInit_Success;
}


void  read_imei(int slotid)
{
    diag_data[2] = 0x01;
    diag_data[6] = (unsigned char)slotid;
    
    diag_callback_send_data(MSM, diag_data, DIAG_BUF_LEN);
}    


void write_imei(int slotid,  const char *imei)
{
    int i,j;

    diag_data[2] = 0x02;       //write
    diag_data[6] = (unsigned char)slotid;

    diag_data[8] = 0x08;
    diag_data[9] = (unsigned char)((imei[0] - 0x30) * 0x10 + 0x0A);
    
    for(i = 1, j=1; i<14;) {
        diag_data[9+j] = (imei[i+1] - 0x30) * 0x10 + (imei[i] - 0x30);
        i = i + 2;
        j = j + 1;
    }

	for (i = 0; i < 0x20; i++) {
		DIAG_LOGE("%02x\t", diag_data[i]);
	}

    diag_callback_send_data(MSM, diag_data, DIAG_BUF_LEN);
}    




int
main(int argc, char *argv[])
{
    int flags, opt;
    int slotid, tfnd;
    char *imei = NULL;

    slotid = 0;
    tfnd = 0;
    flags = 0;
    while ((opt = getopt(argc, argv, "i:s:")) != -1) {
        switch (opt) {           
        case 's':
            slotid = atoi(optarg);
            printf("slotid=%d\n", slotid);
            tfnd = 1;
            break;

        case 'i':
            imei = optarg;
            printf("imei=%s\n", imei);
            break;
            
        default: /* '?' */
            fprintf(stderr, "Usage: %s [-s slotid] [-m] mode\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    printf("argc=%d, flags=%d; tfnd=%d; optind=%d\n", argc, flags, tfnd, optind);

    //if (optind >= argc) {
    //    fprintf(stderr, "after -m need a r/w argument\n");
    //    exit(EXIT_FAILURE);
    //}

    //printf("mode argument = %s\n", argv[optind]);

    init_nv();
    if(init_diag()) {
        if(imei == NULL) {
            read_imei(slotid);
        } else {
            printf("write_imei=%s\n", imei);
            write_imei(slotid, imei);
        }
        

        sleep(2);
        while (1) {
            /* Infinte loop to keep the app awake
            to receive data */
        }

        Diag_LSM_DeInit();
        exit(EXIT_SUCCESS);
    } else {
		DIAG_LOGE("Diag LSM Init failed, Exiting... err:%d", errno);
		exit(EXIT_FAILURE);
    }        
}






