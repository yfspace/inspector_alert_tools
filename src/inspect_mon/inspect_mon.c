
// derived from example hid code at http://www.signal11.us/oss/hidapi/ 

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

#include <hidapi/hidapi.h>


#define LOG_NONE   0
#define LOG_FATAL  1
#define LOG_ERROR  2
#define LOG_WARN   3
#define LOG_NOTICE 4
#define LOG_DEBUG  5
#define LOG_MAX    6

#define MAX_READ_BUF_SIZE 4096

#define NULL_SAFE(x)    (x == NULL ? (const char *)    "(NULL)" : (const char *)    x)
#define NULL_SAFE_LS(x) (x == NULL ? (const wchar_t *) "(NULL)" : (const wchar_t *) x)

#define IMPORT_BE_CHAR_TO_HOST_32(result, source, ofs) \
   (result = (((unsigned long) source[3 + ofs]) << 24) + \
             (((unsigned long) source[2 + ofs]) << 16) + \
             (((unsigned long) source[1 + ofs]) << 8) +  \
             ((unsigned long) source[0 + ofs]), result)

/* timeval_subtract() from http://www.gnu.org/software/libc/manual/html_node/Elapsed-Time.html */
/* Subtract the ‘struct timeval’ values X and Y,
   storing the result in RESULT.
   Return 1 if the difference is negative, otherwise 0. */
int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y)
{
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}

unsigned long long timeval_subtract_msec(struct timeval *x, struct timeval *y)
{
   struct timeval result;
   if (timeval_subtract(&result, x, y)) {
     // x - y is negative..
     return 0;
   }
    
   unsigned long long result_msec = (result.tv_sec * 1000) + (result.tv_usec / 1000);
   return result_msec;
}

static const char *LOG_LEVELS[] = {"LOG_NONE", "ERROR", "WARN", "debug"};

/* global log level */
int mLogLevel = LOG_ERROR;
int mDebugCode = 1; 

#define log(log_level, msg, args...) \
  ((log_level <= mLogLevel) ? _log(log_level, __FILE__, __LINE__, __FUNCTION__, msg, ##args), 0 : 0)

void _log(int           log_level, 
          const char   *file, 
          unsigned int  line, 
          const char   *function,
          const char   *msg,  
          ... ) 
{
    if (log_level > LOG_MAX) log_level = LOG_MAX;
    
    if (mDebugCode) {
        fprintf(stderr, "[%s:%u (%s)] %s - ", file, line, function, LOG_LEVELS[log_level]);
    } else { 
        fprintf(stderr, "%s - ", LOG_LEVELS[log_level]);
    }
    
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr, "\n");
    
    if (log_level == LOG_FATAL) {
        fprintf(stderr, "ABORTING\n");
        fflush(stderr);
        exit(-1);
    }
}



// find the hid device with the inspector 
// this is used when a path is not specified. 
hid_device *open_device(unsigned short vendor_id, 
                        unsigned short product_id, 
                        const wchar_t *serial_number) 
{
    hid_device *handle = NULL;
    log(LOG_DEBUG, "Attempting to open device %us %us %ls", 
        vendor_id, product_id, NULL_SAFE_LS(serial_number));
    
    // Open the device using the VID, PID,
    // and optionally the Serial number.
    handle = hid_open(vendor_id, product_id, serial_number);
    
    if (handle != NULL) {
        log(LOG_NOTICE, "Successfully opened device %us %us %ls",
            vendor_id, product_id, NULL_SAFE_LS(serial_number));
    } else { 
        log(LOG_DEBUG, "Unable to open device %us %us %ls",
            vendor_id, product_id, NULL_SAFE_LS(serial_number));
    }
    return handle;
}

hid_device *find_and_open_device(const char *path) 
{
    hid_device *handle = NULL;
    if (path != NULL) {
        handle = hid_open_path(path);
    } else {
        // try known device ids
        handle = open_device(0x1781, 0x08e9, NULL); // Inspector USB, purchased Jan 2017
        /* if (handle == NULL) handle = open_device(0xffff, 0xffff, NULL); ... */
    } 
    if (handle == NULL) {
        log(LOG_FATAL, "Unable to find/open device (path=%s)",
            NULL_SAFE(path));
    }
    return handle;
}

void list_all_devices() 
{
    // Enumerate and print the HID devices on the system
    struct hid_device_info *devs, *cur_dev;

    devs = hid_enumerate(0x0, 0x0);
    cur_dev = devs;	
    while (cur_dev) {
        printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls",
                cur_dev->vendor_id, cur_dev->product_id, 
                NULL_SAFE(cur_dev->path), NULL_SAFE_LS(cur_dev->serial_number));
        printf("\n");
        printf("  Manufacturer: %ls\n", NULL_SAFE_LS(cur_dev->manufacturer_string));
        printf("  Product:      %ls\n", NULL_SAFE_LS(cur_dev->product_string));
        printf("\n");
        cur_dev = cur_dev->next;
    }
    hid_free_enumeration(devs);
}

int __hid_api_ret = 0;
#define CHECK_HIDAPI_CALL(x) \
  ((0 > (__hid_api_ret = (x))) ? log(LOG_ERROR, "CALL FAILED : %s" , #x ), __hid_api_ret : __hid_api_ret)

void show_device_info(hid_device *handle) 
{
    int res; 
    #define MAX_STR 255
    wchar_t wstr[MAX_STR];
    
    // Read the Manufacturer String
    wstr[0] = 0;
    res = CHECK_HIDAPI_CALL(hid_get_manufacturer_string(handle, wstr, MAX_STR));
    printf("Manufacturer String: %ls\n", wstr);

    // Read the Product String
    wstr[0] = 0;
    res = CHECK_HIDAPI_CALL(hid_get_product_string(handle, wstr, MAX_STR));
    printf("Product String: %ls\n", wstr);

    // Read the Serial Number String
    wstr[0] = 0;
    res = hid_get_serial_number_string(handle, wstr, MAX_STR); // let it fail.
    printf("Serial Number String: %ls", wstr);
    printf("\n"); 
}

int read_feature_rept(hid_device *handle, unsigned char rept_num) 
{
    unsigned char report_buf[4000];
    memset(report_buf, 0, sizeof(report_buf));
    
    report_buf[0] = rept_num;
    int report_size = hid_get_feature_report(handle, report_buf, sizeof(report_buf));
    
    if (report_size <= 0) {
        log(LOG_FATAL, "Unable to read report %u (%x), returned %i: %s", 
            rept_num, rept_num, report_size, NULL_SAFE(hid_error(handle)));
    }
    
    // show the data
    printf("*** READ REPORT %u (%u) Results: (%i bytes) ***\n",
           rept_num, rept_num, report_size);
    
    printf("r_buf:         ");
    int i;
    for (i = 0; i < report_size; i++) {
        printf("%02x ", report_buf[i]);
        if (i && (((i+1) % 16) == 0)) {
            printf("\n               "); 
        }
    }
    
    int fd = open("desc_rept", O_CREAT | O_TRUNC | O_WRONLY, 0755);
    if (fd != -1) {
        log(LOG_NOTICE, "Writing desc_rept file");
        int ret = write(fd, report_buf, report_size);
        close(fd);
    }
    printf("\n"); 
    
}


enum mode_e { 
    MODE_UNKNOWN     = 0,
    MODE_TOTAL_TIMER = 1,
    MODE_CPM         = 2,
    MODE_MR_PER_HOUR = 3,
    MODE__MAX        = 3 // same as highest, and in order
};
          
typedef struct { 
    // this first set should be initialized by caller prior to read
    unsigned int       in_counter;        //!< read packet number (our end),
    unsigned long long in_ll_counter;     //!< non-unique reads (our end).
    int                num_bytes_to_read; //!< nuumber of bytes to read
    struct timeval     prev_readtime;     //!< previous read time
    
    unsigned char      r_buf[MAX_READ_BUF_SIZE]; //!< raw read data
    char               r_bytes_read;             //!< bytes read
    
    struct timeval     r_readtime;        //!< read time
    enum mode_e        r_mode;            //!< switch mode 
    unsigned long      r_counter;         //!< read counter
    unsigned long long r_ll_counter;      //!< read ll counter (for this read only)
    unsigned long      r_clicks;          //!< number of clicks read
    unsigned long      r_uR_per_hr;       //!< uR/Hr
    unsigned long      r_cpm;             //!< CPM
    unsigned long long r_time_span_ms;    //!< time span of sample

} read_data_pkt_t; 

const char *get_mode_str(enum mode_e mode) 
{
    static const char *names[] = {"MODE_UNKNOWN", "MODE_TOTAL_TIMER", "MODE_CPM", "MODE_mR/hr" };
    if (((int) mode > (int) MODE__MAX) ||
        ((int) mode < 0)) {
        mode = MODE_UNKNOWN;
    }
    return names[(int)mode];
}

unsigned long get_r_counter(const unsigned char *buf) {
    //unsigned long val = 0;
    //return IMPORT_BE_CHAR_TO_HOST_32(val, buf);
    return (unsigned long) buf[4];
} 

enum mode_e get_r_mode(const unsigned char *buf) {
    switch(buf[9]) {
        case 0x02: return MODE_MR_PER_HOUR;
        case 0x00: return MODE_CPM;
        case 0x14: return MODE_TOTAL_TIMER;
        default: return MODE_UNKNOWN;
    }
}



unsigned long get_r_clicks(const unsigned char *buf) {
    unsigned long val = 0;
    IMPORT_BE_CHAR_TO_HOST_32(val, buf, 0);
    if (val > 0xffffff00) // handle temporary negative values
        val = 0;
    return val;
} 

unsigned long get_r_uR_per_hr(const unsigned char *buf) {
    unsigned long val = 0;
    return IMPORT_BE_CHAR_TO_HOST_32(val, buf, 5);
} 

unsigned long get_r_cpm(const unsigned char *buf) {
    unsigned long val = 0;
    return IMPORT_BE_CHAR_TO_HOST_32(val, buf, 5);
} 

int read_data_blocking(hid_device *handle, 
                       read_data_pkt_t *read_data_pkt)
{
    // set handle to blocking mode
    CHECK_HIDAPI_CALL(hid_set_nonblocking(handle, 0)); 
    
    struct timespec ts;
    ts.tv_sec  = 0;
    //ts.tv_nsec = 500000000; // 0.5 sec
    ts.tv_nsec = 5000000; // 0.005 sec
    
    struct timeval now_tv;
    
    
    // I noticed 8 null bytes sent to the inspector on a regular basis by the 
    // Observer_USB software... 
    // but it seems like it has no effect.
    const unsigned char zero_buf[8] = {0,0,0,0,0,0,0,0};
    //CHECK_HIDAPI_CALL(hid_write(handle, zero_buf, 8));

  
    while (1) { 
        // read some data
        read_data_pkt->r_bytes_read = 
            CHECK_HIDAPI_CALL(hid_read(handle, read_data_pkt->r_buf, 
                              read_data_pkt->num_bytes_to_read));
        
        read_data_pkt->r_ll_counter ++;
    
        // TODO: any better way than this hardcoded mess?
        if (read_data_pkt->r_bytes_read = 15) {
            unsigned long read_counter = get_r_counter(read_data_pkt->r_buf);
            if (read_counter == read_data_pkt->r_counter) {
                // we know the device seems to update its data every second, and
                // does not seem to work very well unless we poll it soon after 
                // it updates it's data.. It is strange that the read operation
                // does not block until we have new data ready.... perhaps if 
                // I knew the write report parameters, I could modify this to 
                // properly handle this... 
                
                // Actually, w/o sending any reports, I am getting lots of data back, 
                // but if I am not constantly reading (even though it is returning
                // the same data back repeately), the data seems to start queueing
                // and the numbers do not match the ones displayed on the device.
                // The logic below works, however it does start queueing, so for now
                // we will read every 5 ms.
                ts.tv_nsec = 5ULL * 1000000ULL; // 5 ms
                
#ifdef LESS_READS_BUT_QUEUEING

                // NOTE: it seems like this logic below cant flush the device.. and
                //       the numbers get
                // ok., to reduce the CPU and spin load, we will read every
                //   950ms + last read - now if < 950 ms since last read 
                //   20 ms if it has been > 950ms since last read
                //   10 ms if it has been > 980ms since last read
                //   5  ms if it has been > 990ms since last read
                // over time this should eventually limit our reads to about 5 / sec
                gettimeofday(&now_tv, NULL);
                unsigned long long since_last_ms = timeval_subtract_msec(
                   &now_tv,
                   &read_data_pkt->prev_readtime);
                if (since_last_ms < 950) {
                    ts.tv_nsec = ((950ULL - since_last_ms) * 1000000ULL);
                } else if (since_last_ms < 980) {
                    ts.tv_nsec = 20ULL * 1000000ULL; // 20 ms
                } else if (since_last_ms < 990) { 
                    ts.tv_nsec = 10ULL * 1000000ULL; // 10 ms
                } else {
                    ts.tv_nsec = 5ULL * 1000000ULL; // 5 ms
                }
               // fprintf(stderr, "...sleeping %llu ns, last update %llu ms ago\n", (unsigned long long) ts.tv_nsec, since_last_ms);
#endif
                nanosleep(&ts, NULL);
            } else { 
                gettimeofday(&read_data_pkt->r_readtime, NULL);
                read_data_pkt->r_time_span_ms = timeval_subtract_msec(
                   &read_data_pkt->r_readtime,
                   &read_data_pkt->prev_readtime);
                read_data_pkt->r_counter = read_counter;
                read_data_pkt->r_clicks  = get_r_clicks(read_data_pkt->r_buf);
                read_data_pkt->r_mode    = get_r_mode(read_data_pkt->r_buf); 
                switch (read_data_pkt->r_mode) {
                    case MODE_MR_PER_HOUR: 
                        read_data_pkt->r_uR_per_hr = get_r_uR_per_hr(read_data_pkt->r_buf);
                        break;
                    case MODE_CPM:
                        read_data_pkt->r_cpm = get_r_cpm(read_data_pkt->r_buf);
                        break;
                    default:
                        break;
                }
                break; // break while loop
            }
        }  else {
            // error.. just break out read with error.. 
           break; // break while loop
        }
    }
    return ((read_data_pkt->r_bytes_read >= 0) ? 0 : -1);
}

int show_data(const read_data_pkt_t *read_data_pkt)
{
   printf("------ READ ------\n");
   printf("in_counter:    %u (non-unique reads: %llu, non-unique reads this sample: %llu)\n",
           read_data_pkt->in_counter,
           read_data_pkt->in_ll_counter, 
           read_data_pkt->r_ll_counter);
   printf("timespan:      %llu msecs\n", read_data_pkt->r_time_span_ms);
   printf("bytes_read:    %i\n", read_data_pkt->r_bytes_read);
   printf("sample:        %lu\n", read_data_pkt->r_counter);
   printf("clicks:        %lu\n", read_data_pkt->r_clicks);
   printf("mode:          %s\n", get_mode_str(read_data_pkt->r_mode));
   
   switch (read_data_pkt->r_mode) {
       case MODE_MR_PER_HOUR:
           printf("mR/Hr read:    %.3g\n", (double) read_data_pkt->r_uR_per_hr / 1000 );
           break;
       case MODE_CPM:
           printf("CPM:           %lu\n", read_data_pkt->r_cpm);
           break;
       default:
           break;
   }

   if (read_data_pkt->r_bytes_read <= 0) {
       return read_data_pkt->r_bytes_read;
   }
   printf("r_buf:         ");
   int i;
   for (i = 0; i < read_data_pkt->r_bytes_read; i++) {
       printf("%02x ", (unsigned char) read_data_pkt->r_buf[i]);
   }
   printf("\n"); 
}


int main(int argc, char* argv[])
{
    
    if (hid_init() != 0) { 
        log(LOG_FATAL, "Unable to initialize hid library");
    }
    
    // todo: parse args properly (including -v, and allow vendor,product,serial to be specified too. )
    if ((argc == 2) && (!strcmp(argv[1], "-l"))) {
        list_all_devices();
        exit(0);
    }
    
    unsigned long feature_rept_num = 0;
    int do_read_feature_rept = 0;
    if ((argc == 3) &&
        (!strcmp(argv[1], "-r"))) {
        char *check = argv[2];
        feature_rept_num = strtol(argv[2], &check, 0);
        if ((check == NULL) || (*check != 0) || (feature_rept_num > 0xff)) {
            log(LOG_FATAL,"Invalid Record: %s", argv[2]);
        }
        do_read_feature_rept = 1;
    }
    
    const char *path = NULL;
    if ((argc == 3) && 
        (!strcmp(argv[1], "-p"))) {
        path = argv[2];
    }
    
    // open the device
    hid_device *handle = find_and_open_device(path);
    
    // show information about opened device
    show_device_info(handle);
    
    if (do_read_feature_rept) {
        read_feature_rept(handle, feature_rept_num);
        hid_close(handle);
        exit(0);
    }
    
    unsigned long read_pkt_num = 0UL;
    read_data_pkt_t read_data_pkt;
    unsigned long last_counter = 0UL;
    struct timeval last_read;
    unsigned long long in_ll_counter = 0ULL;
    gettimeofday(&last_read, NULL);
    while (1) {
        memset(&read_data_pkt, 0, sizeof(read_data_pkt));
        read_data_pkt.in_counter = ++read_pkt_num;
        read_data_pkt.in_ll_counter = in_ll_counter; 
        read_data_pkt.num_bytes_to_read = 512; // TODO: figure this out from the device itself!
        read_data_pkt.r_counter = last_counter;
        read_data_pkt.prev_readtime = last_read;
        read_data_blocking(handle, &read_data_pkt);
        if (read_data_pkt.r_bytes_read > 0) {
            last_counter = read_data_pkt.r_counter;
            last_read = read_data_pkt.r_readtime;
            in_ll_counter += read_data_pkt.r_ll_counter;
        }
        show_data(&read_data_pkt);
        
    }
    
    hid_close(handle);
    
    return 0;
}

#ifdef foo
    
    
	int res;
	unsigned char buf[65];
	#define MAX_STR 255
	wchar_t wstr[MAX_STR];
	hid_device *handle;
	int i;


	// Open the device using the VID, PID,
	// and optionally the Serial number.
	handle = hid_open(0x4d8, 0x3f, NULL);

	

	// Send a Feature Report to the device
	buf[0] = 0x2; // First byte is report number
	buf[1] = 0xa0;
	buf[2] = 0x0a;
	res = hid_send_feature_report(handle, buf, 17);

	// Read a Feature Report from the device
	buf[0] = 0x2;
	res = hid_get_feature_report(handle, buf, sizeof(buf));

        return 0;
        
	// Print out the returned buffer.
	printf("Feature Report\n   ");
	for (i = 0; i < res; i++)
		printf("%02hhx ", buf[i]);
	printf("\n");

	// Set the hid_read() function to be non-blocking.
	hid_set_nonblocking(handle, 1);

	// Send an Output report to toggle the LED (cmd 0x80)
	buf[0] = 1; // First byte is report number
	buf[1] = 0x80;
	res = hid_write(handle, buf, 65);

	// Send an Output report to request the state (cmd 0x81)
	buf[1] = 0x81;
	hid_write(handle, buf, 65);

	// Read requested state
	res = hid_read(handle, buf, 65);
	if (res < 0)
		printf("Unable to read()\n");

	// Print out the returned buffer.
	for (i = 0; i < res; i++)
		printf("buf[%d]: %d\n", i, buf[i]);

	return 0;
}

#endif