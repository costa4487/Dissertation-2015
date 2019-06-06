#include "libcurl .h" 

char* concat (int count , ...) { 
	va_list ap; 
	int i; 

	//Find required length to store merged string
	int len = 1; // room for NULL 
	va_start(ap , count); 
	for(i =0 ; i<count ; i++) 
		len += strlen (va_arg(ap, char*)); 
	va_end (ap); 

	//Allocate memory to concat strings 
	char *merged = (char*)calloc(sizeof(char),len); 
	int null_pos = 0; 

	// concatenate strings 
	va_start(ap, count); 
	for(i =0 ; i<count ; i++)
	{
		char *s = va_arg(ap, char*); 
		strcpy(merged+null_pos, s);
		null_pos += strlen(s); 
	}
	va_end(ap); 

	return merged; 
} 

struct FtpFile {
	const char *filename; 
	FILE *stream; 
};

static size_t my_fwrite(void *buffer, size_t size, size_t nmemb, void* stream) {
	struct FtpFile *out =(struct FtpFile *)stream;
	if(out && !out->stream) {
		// open file for writing
		out->stream=fopen(out->filename, "wb");
		if(!out->stream)
			return -1; //failure, can’t open file to write
	}
	return fwrite(buffer, size, nmemb, out->stream); 
}


int curl_get(const char* infilename, const char* inURL){ 
	CURL *curl;
	CURLcode res;
	struct FtpFile ftpfile={
		infilename, // name to store the file as if successful 
		NULL
	};
	char* fullURL = concat(2,inURL,infilename);

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL,fullURL);
		curl_easy_setopt(curl, CURLOPT_USERPWD, "daniel:password");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);// Define our callback to get called when there’s data to be written
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);// Set a pointer to our struct to pass to the callback

		//execute defined cURL
		res = curl_easy_perform(curl); 

		//Error handling
		if(CURLE_OK != res) fprintf(stderr, "cURL get : Error %s\n", curl_easy_strerror(res));
	}

	//Cleanup
	if(ftpfile.stream) fclose(ftpfile.stream);
	curl_easy_cleanup(curl);
	curl_global_cleanup();
	free(fullURL);
	return (int)res;
}


static size_t throw_away(void *ptr, size_t size, size_t nmemb, void *data){
	(void)ptr;
	(void)data;
	return (size_t)(size * nmemb);
}


time_t curl_get_info(const char* infilename, const char* inURL){ 
	CURL *curl;
	CURLcode res;
	long filetime = -1;
	time_t file_time = (time_t)0;
	char* fullURL = concat (2,inURL,infilename); 
	const char *filename = strrchr(fullURL, '/') + 1;

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, fullURL);
		curl_easy_setopt(curl, CURLOPT_USERPWD, "daniel:password");
		curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);//Don’t download the file data
		curl_easy_setopt(curl, CURLOPT_FILETIME, 1L); //Ask for filetime
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, throw_away); //No header output

		//Suppress standard output (otherwise dumps file info) 
		int normal, bit_bucket;
		fflush(stdout); 
		normal = dup(1);
		bit_bucket = open("/dev/ null", O_WRONLY);
		dup2(bit_bucket, 1);
		close(bit_bucket);
		//execute defined cURL
		res = curl_easy_perform(curl);
		//restore standard output
		fflush(stdout);
		dup2(normal, 1);
		close(normal);

		if(CURLE_OK == res) {
			//file modificaiton time processing
			res = curl_easy_getinfo(curl, CURLINFO_FILETIME, &filetime);
			if((CURLE_OK == res) && (filetime >= 0)) {
				file_time = (time_t)filetime;
			}
		}
		// Error handling
		else fprintf(stderr, "cURL get info: Error %s\n", curl_easy_strerror(res));
	}

	//cleanup
	curl_easy_cleanup(curl);
	free(fullURL);
	curl_global_cleanup();

	//Return file modification time
	return file_time;
}