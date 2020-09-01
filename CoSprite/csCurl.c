#include "csCurl.h"

/** \brief Initializes curl globally, and opens a handle in the globalCurl object.
 *
 * \param flags - curl_global_init flags (typically CURL_GLOBAL_ALL)
 * \param certPath - the full path (including filename) to your cert *.pem file
 */
void initCoSpriteCurl(long flags, char* certPath)
{
    globalCurl.retCode = curl_global_init(flags);
    if(globalCurl.retCode != CURLE_OK)
    {
        fprintf(stderr, "curl_global_init() failed: %s\n", curl_easy_strerror(globalCurl.retCode));
        globalCurl.online = false;
    }
    else
        globalCurl.online = true;

    initCSCurl(&globalCurl, certPath);
}

/** \brief Initializes a curl handle. initCoSpriteCurl() must be called prior to this.
 *
 * \param handle - your csCurl pointer to be filled in (defaults to a GET)
 */
void initCSCurl(csCurl* handle, char* certPath)
{
    handle->retCode = CURLE_OK;
    handle->handle = curl_easy_init();
    if (handle->handle == NULL)
    {
        fprintf(stderr, "curl_easy_init() failed\n");
        handle->online = false;
    }
    else
        handle->online = true;

    curl_easy_setopt(handle->handle, CURLOPT_FOLLOWLOCATION, 1L);  //follow redirects
    curl_easy_setopt(handle->handle, CURLOPT_VERBOSE, 1L);  //show verbose
    curl_easy_setopt(handle->handle, CURLOPT_CAINFO, certPath);  //give curl a list of good certs
    curl_easy_setopt(handle->handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");     // some servers don't like requests that are made without a user-agent field, so we provide one
    curl_easy_setopt(handle->handle, CURLOPT_HTTPGET, 1L);  //set the user's request option to true (in this case, a GET)
    curl_easy_setopt(handle->handle, CURLOPT_SSL_SESSIONID_CACHE, 0L);  //i think we need to remove this
}

/** \brief
 *
 * \param handle - address of csCurl struct
 * \param url - the url you want to GET from, including any options or parameters
 * \param outputString - allocated memory that the data will be appended to (make it big enough!)
 */
void csCurlPerformEasyGet(csCurl* handle, char* url, char* outputString)
{
    curl_easy_setopt(handle->handle, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(handle->handle, CURLOPT_URL, url);

    curl_easy_setopt(handle->handle, CURLOPT_WRITEFUNCTION, performEasyGetCallback);

    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(handle->handle, CURLOPT_WRITEDATA, (void*) outputString);

    /* Perform the request, res will get the return code */
    handle->retCode = curl_easy_perform(handle->handle);

    if(handle->retCode != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(handle->retCode));
        //handle->online = false;
    }
}

/** \brief
 *
 * \param handle - address of csCurl struct
 * \param url - the url you want to POST to
 * \param data - parameters to be passed like "x=1&y=2"
 */
void csCurlPerformEasyPost(csCurl* handle, char* url, char* data)
{
    curl_easy_setopt(handle->handle, CURLOPT_POST, 1L);

    curl_easy_setopt(handle->handle, CURLOPT_POSTFIELDS, data);

    curl_easy_setopt(handle->handle, CURLOPT_URL, url);

    handle->retCode = curl_easy_perform(handle->handle);

    if(handle->retCode != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(handle->retCode));
        //handle->online = false;
    }
}

/** \brief Internal usage only; callback for csCurlPerformEasyGet()
 * \param ptr - data gotten
 * \param size - size of type returned
 * \param nmemb - number of members
 * \param userdata - any data you want to access
 * \return size_t - if not returning size * nmemb, then an error must have occured
 */
size_t performEasyGetCallback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    char* outputString = (char*) userdata;
    strcat(outputString, ptr);
    return size * nmemb;
}

/** \brief Clean up a csCurl struct
 *
 * \param handle - handle to curl struct you want cleaned up
 */
void destroyCSCurl(csCurl* handle)
{
    curl_easy_cleanup(handle->handle);
    handle->handle = NULL;
    handle->online = false;
}

/** \brief Closes CoSprite's Curl extension
 */

void closeCoSpriteCurl()
{
    destroyCSCurl(&globalCurl);
    curl_global_cleanup();
}
