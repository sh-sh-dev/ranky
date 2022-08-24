    #define _GNU_SOURCE

    #include <stdio.h>
    #include <pthread.h>
    #include <string.h>
    #include <stdlib.h>
    #include <jansson.h>
    #include <curl/curl.h>
    #include <libpq-fe.h>
    #include <time.h>

    #include <bsd/string.h>

    #define NUM_THREADS 5
    #define NUM_COUNTRY 20
    #define NUM_GENRE 26
    #define NUM_MAC_GENRE 23
    #define NUM_CHARTS 3

    #define NUM_TOTAL_ITEMS   NUM_COUNTRY*NUM_GENRE*NUM_CHARTS
    #define NUM_TOTAL_MAC_ITEMS   NUM_COUNTRY*NUM_MAC_GENRE*NUM_CHARTS
    #define QUERY_STRING_BUFFER 10000

    #define NUM_STORES 3
    #define STORE_TYPE_IPHONE 0
    #define STORE_TYPE_IPAD 1
    #define STORE_TYPE_MAC 2

    pthread_mutex_t allArrayLock;

    int storeType;

    struct MemoryStruct
    {
        char *memory;
        size_t size;
    };

    typedef enum
    {
        top_free = 0,
        top_paid,
        top_grossing
    }chart_type_t;

    typedef struct UrlObject
    {
        const char *iso2char;
        int genre;
        chart_type_t chartType;
    }UrlObject_t;

    struct StackObject
    {
        UrlObject_t urlObject;
        struct StackObject *NEXT;
    }*HEAD;

    /*
    const char * const countryCodes[NUM_COUNTRY] =
    {
    "CY",
    "US",
    "IN"
    };
    */

    // 2 (a)   #30
    //const char * const countryCodes[NUM_COUNTRY] = {"AR","AT","BH","BY","BE","VG","BN","BG","KH","KY","CL","CO","CR","HR","CY","CZ","DO","EC","EG","GR","HK","HU","IS","IN","ID","IE","IL","JM","JO","NZ"};

    // 2 (b)   #30
    //const char * const countryCodes[NUM_COUNTRY] = {"KW","LB","LT","LU","MO","MY","OM","PA","PE","PH","PL","PT","QA","RO","SA","SC","SG","SK","SI","ZA","LK","CH","TH","TN","TR","UA","AE","UY","VE","VN"};

    // 1    #20
    const char * const countryCodes[NUM_COUNTRY] = {"AU","BR","CA","CN","FR","DE","IT","JP","MX","NL","RU","ES","SE","GB","US","KR","FI","DK","TW","NO"};

    // 3 (a)   #37
    //const char * const countryCodes[NUM_COUNTRY] = {"AL","DZ","AO","AI","AG","AM","AZ","BS","BB","BZ","BJ","BM","BT","BO","BW","BF","CV","TD","CG","DM","SV","EE","FJ","GM","GH","GD","GT","GW","GY","HN","KZ","KE","KG","LA","LV","LR","MK"};

    // 3 (b)   #38
    //const char * const countryCodes[NUM_COUNTRY] = {"MG","MW","ML","MT","MR","MU","FM","MD","MN","MS","MZ","NA","NP","NI","NE","NG","PK","PW","PG","PY","SN","SL","SB","KN","LC","VC","SR","SZ","ST","TJ","TZ","TT","TM","TC","UG","UZ","YE","ZW"};


    //  http://www.apple.com/itunes/affiliates/resources/documentation/genre-mapping.html
    //  6000 Business
    //  6001 Weather
    //  6002 Utilities
    //  6003 Travel
    //  6004 Sports
    //  6005 Social Networking
    //  6006 Reference
    //  6007 Productivity
    //  6008 Photo & Video
    //  6009 News
    //  6010 Navigation
    //  6011 Music
    //  6012 Lifestyle
    //  6013 Health & Fitness
    //  6014 Games
    //  6015 Finance
    //  6016 Entertainment
    //  6017 Education
    //  6018 Books
    //  6020 Medical
    //  6021 Newsstand=> Magazine & Newspaper
    //  6023 Food & Drink
    //  6024 Shopping

    // these 2 are not there
    //  6022 Catalogs
    //  6025 Stickers

    //for mac: changed the category ids to same as ios
    // 18,21,23 not found for mac
    // new ids:
    // 6026 Developer Tools
    // 6027 Graphics & Design
    // Update:
    // 6026 and 6027 also there for iOS now.
    // universal apps can now be Mac also. one app id, all three: iphone, ipad, mac

    //int genres[NUM_GENRE] = {0,6014};
    int genres[NUM_GENRE] = {36,6000,6001,6002,6003,6004,6005,6006,6007,6008,6009,6010,6011,6012,6013,6014,6015,6016,6017,6018,6020,6021,6023,6024,6026,6027};
    //int genresMac[NUM_MAC_GENRE] = {39,12001,12002,12003,12004,12005,12006,12007,12008,12010,12011,12012,12013,12014,12015,12016,12017,12018,12019,12020,12021,12022};
    int genresMac[NUM_MAC_GENRE] = {36,6000,6001,6002,6003,6004,6005,6006,6007,6008,6009,6010,6011,6012,6013,6014,6015,6016,6017,6020,6024,6026,6027};
    chart_type_t chartTypes[NUM_CHARTS] = {top_free , top_paid , top_grossing};

    const char * const storeChartURLSlugs[NUM_STORES][NUM_CHARTS] = {

                                        {"&name=FreeAppsV2", "&name=PaidApplications", "&name=AppsByRevenue"},
                                        {"&name=FreeIpadApplications", "&name=PaidIpadApplications", "&name=IpadAppsByRevenue"},
                                        {"&name=FreeMacAppsV2", "&name=PaidMacAppsV2", "&name=MacAppsByRevenueV2"}
    };
    //for iOS FreeApplications (old) gives list with Games in it.
    //  FreeAppsV2 gives app only list. Since the app store itself has separate sections for it.
    // PaidMacAppsV2 and MacAppsByRevenueV2 do not exist yet but if I leave it to older values (PaidMacApps and MacAppsByRevenue),
    // it fetches ios apps list. so keeping V2, gives empty list as of now. might start working.

    int push_to_object_stack(UrlObject_t newObject)
    {
        struct StackObject *pushObj = malloc(sizeof(struct StackObject));
        if(pushObj == NULL)
        {
            printf("ERR : Not enough memory.malloc returned NULL while pushing to stack.\n");
            return 1;
        }
        pushObj->urlObject = newObject;
        pushObj->NEXT = HEAD;
        HEAD = pushObj;
        return 0;
    }

    UrlObject_t pop_from_object_stack()
    {
        UrlObject_t returnObj = {NULL,0,0};
        if(HEAD == NULL)
        {
            printf("WARN : Stack is EMPTY!!\n");
            return returnObj;
        }
        returnObj = HEAD->urlObject;
        struct StackObject *popped = HEAD;
        HEAD = HEAD->NEXT;
        free(popped);

        return returnObj;
    }
    int build_allobjects_stack()
    {
        HEAD = NULL;
        int i = 0;
        int j = 0;
        int k = 0;
        int count_all = 0;
        UrlObject_t oneObject;

        int numGenre = NUM_GENRE;
        int numTotalItems = NUM_TOTAL_ITEMS;

        if(STORE_TYPE_MAC == storeType)
        {
            numGenre = NUM_MAC_GENRE;
            numTotalItems = NUM_TOTAL_MAC_ITEMS;
        }

        for(k = 0; k < NUM_CHARTS ; k++)
        {
            for(j = 0; j < numGenre ; j++)
            {
                for(i = 0 ; i < NUM_COUNTRY ; i++)
                {
                    if(count_all < numTotalItems)
                    {
                        if(STORE_TYPE_MAC == storeType)
                        {
                            oneObject.genre = genresMac[j];
                        }
                        else
                        {
                            oneObject.genre = genres[j];
                        }

                        oneObject.chartType = chartTypes[k];
                        oneObject.iso2char = countryCodes[i];
                        if(push_to_object_stack(oneObject))
                        {
                            printf("ERR : error in pushing object to stack.\n");
                            return 1;
                        }
                        count_all++;
                    }
                    else
                    {
                        printf("ERR : Should not come here!!! End of allUrlObjects");
                        return 1;
                    }
                }
            }
        }
        if(count_all == numTotalItems)
        {
            printf("INFO : Stack built successfully\n");
        }
        else
        {
            printf("ERR : Stack NOT built fully!! Only %i items.\n",count_all);
            return 1;
        }

        return 0;
    }

    void exit_nicely(PGconn *conn)
    {
        PQfinish(conn);
        exit(1);
    }

    UrlObject_t get_one_url_to_fetch()
    {
        int i = 0;
        UrlObject_t freeUrl;

        pthread_mutex_lock(&allArrayLock);

        freeUrl = pop_from_object_stack();

        pthread_mutex_unlock(&allArrayLock);

        return freeUrl;
    }

    static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
        size_t realsize = size * nmemb;
        struct MemoryStruct *mem = (struct MemoryStruct *)userp;

        mem->memory = realloc(mem->memory, mem->size + realsize + 1);
        if(mem->memory == NULL) {
            /* out of memory! */
            printf("ERR : not enough memory (realloc returned NULL)\n");
            return 0;
        }

        memcpy(&(mem->memory[mem->size]), contents, realsize);
        mem->size += realsize;
        mem->memory[mem->size] = 0;

        return realsize;
    }

    // size_t printHeaders(char* b, size_t size, size_t nitems, void *userdata) {
    //     size_t numbytes = size * nitems;
    //     printf("%.*s\n", numbytes, b);
    //     return numbytes;
    // }

    static void *pull_one_url(CURL *curl, void *url, const char *iso2char, int genre, chart_type_t chartType, PGconn *threadPGConn)
    {
        struct MemoryStruct chunk;

        chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
        chunk.size = 0;    /* no data at this point */

        CURLcode res;
        long code;

        char insertQueryString[QUERY_STRING_BUFFER];
        size_t num_lcpy;

        json_t *root;
        json_error_t error;

        /* send all data to this function  */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

        /* we pass our 'chunk' struct to the callback function */
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        //curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, printHeaders);

        curl_easy_setopt(curl, CURLOPT_URL, url);

        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L); // timeout in seconds, passed as long

        res = curl_easy_perform(curl);

        /* check for errors */
        if(res != CURLE_OK)
        {
            printf("ERR : curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
            printf("ERR : curl error code : %d\n",res);
            return (void *)1;
        }
        else
        {
            /*
            * Now, our chunk.memory points to a memory block that is chunk.size
            * bytes big and contains the remote file.
            *
            * Do something nice with it!
            *
            * You should be aware of the fact that at this point we might have an
            * allocated data block, and nothing has yet deallocated that data. So when
            * you're done with it, you should free() it as a nice application.
            */

            //printf("INFO : %lu bytes retrieved\n", (long)chunk.size);
            // check for response code =200 too
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
            if(code != 200)
            {
                printf("ERR : server responded with code %ld\n\n",code);
                if(code == 403)
                {
                    return (void *)403;
                }
                return NULL;
            }
            if(chunk.memory)
            {
                // using JSON_ALLOW_NUL to allow \u0000 (unicode null). if not used, throws parsing error if the json data contains this char (which was
                // happening sometimes.). available in jansson 2.6 and up
                root = json_loads(chunk.memory , JSON_ALLOW_NUL , &error);
                free(chunk.memory);
            }
            if(!root)
            {
                //printf("ERR : error in parsing: on line %d: %s | Source: %s\n for URL: %s\n", error.line, error.text,error.source,(char *)url);
                printf("ERR : error in parsing: on line %d: %s\n", error.line, error.text);
                //printf("ERR : response: %s\n",chunk.memory);
                return (void *)1;
            }

            if(!json_is_object(root))
            {
                printf("ERR : root is not an object\n");
                json_decref(root);
                return (void *)1;
            }

            json_t *resultIdArray;

            resultIdArray = json_object_get(root, "resultIds");
            if(!json_is_array(resultIdArray))
            {
                printf("WARN : resultIds is not an array for url %s\n",(char *)url);
                json_decref(root);
                return (void *)1;
            }

            if(0 == json_array_size(resultIdArray))
            {
                printf("WARN : no results for url %s\n",(char *)url);
                json_decref(root);
                return (void *)1;
            }
            //printf("URL: %s\n", (char *)url);
            //printf("Array size: %zu %s %i\n", json_array_size(resultIdArray), iso2char, genre);

            //INSERT INTO ranks (time, country, store_type, app_id, chart_type, genre_id, rank) VALUES (NOW(), 'US', 1, '1122334455', 0, 0, 33), (NOW(), 'IN', 2, '1122334477', 0, 6002, 77);
            insertQueryString[0] = '\0';
            num_lcpy = strlcpy(insertQueryString , "INSERT INTO ranks (time,country,store_type,app_id,chart_type,genre_id,rank) VALUES " , sizeof(insertQueryString));

            size_t i;

            for(i = 0; i < json_array_size(resultIdArray); i++)
            {
                json_t *idEntry;
                const char *app_id;

                idEntry = json_array_get(resultIdArray , i);
                if(!json_is_string(idEntry))
                {
                    printf("ERR : idEntry in resultIds at position %zu is not a string.\n",i);
                    json_decref(root);
                    return (void *)1;
                }

                app_id = json_string_value(idEntry);
                int rank = i+1;
                char valueString[45];
                //printf("rank: %i App ID: %s  genre:%i Country:%s\n",rank,app_id,genre,iso2char);
                // (time, country, store_type, app_id, chart_type, genre_id, rank)
                // (NOW(), 'US', 1, '1122334455', 0, 0, 33), (NOW(), 'IN', 2, '1122334477', 0, 6002, 77),....
                // avoid spaces in valueString as it will fill up the insertQueryString.
                sprintf(valueString, "(NOW(),'%s',%i,'%s',%i,%i,%i),", iso2char, storeType, app_id, chartType, genre, rank);

                size_t n_tmp = num_lcpy;
                num_lcpy = strlcpy(insertQueryString + n_tmp , valueString , sizeof(insertQueryString) - n_tmp);
                if(num_lcpy >= sizeof(insertQueryString) - n_tmp)
                {
                    printf("ERR : query string overflow at position %zu\n",i);
                    json_decref(root);
                    return (void *)1;
                }

                num_lcpy += n_tmp;
            }

            //at last position in string replace ',' with '\0'
            insertQueryString[num_lcpy - 1] = '\0';
            //printf("Q: %s\n",insertQueryString);

            PGresult *pgRes = PQexec(threadPGConn, insertQueryString);

            if (PQresultStatus(pgRes) != PGRES_COMMAND_OK) {

                printf("ERR: INSERT failed: %s\n", PQerrorMessage(threadPGConn));
                PQclear(pgRes);
                json_decref(root);
                return (void *)1;
            }
            PQclear(pgRes);
        }
        json_decref(root);
        return NULL;
    }

    void randomString(size_t length, char *randomString)
    {
        static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        size_t charCount = (26*2) + 10;
        int key = 0;

        if(length)
        {
            for (int n = 0; n < length; n++) {
                key = rand() % charCount;
                randomString[n] = charset[key];
            }
            randomString[length] = '\0';
        }
    }

    void *startFetching(void *nothing)
    {
        CURL *curl;
        PGconn *threadPGConn;
        struct timespec startTime;
        struct timespec endTime;
        struct timespec diffTime;
        long oneSec_nsecs = 1000000000; //nano secs
        long oneAndHalfSec_nsecs = 1500000000;
        long halfSec_nsecs = 500000000;
        UrlObject_t urlToFetch = get_one_url_to_fetch();

        curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        struct curl_slist *headerlist = NULL;
        headerlist = curl_slist_append(headerlist, "Cache-Control: no-cache");
        headerlist = curl_slist_append(headerlist, "User-Agent: RankUp Agent/2.0");

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

        //curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        threadPGConn = PQconnectdb("host=localhost port=5432 user=u_rankup password=p_rankup dbname=rankup connect_timeout=30");

        if (PQstatus(threadPGConn) != CONNECTION_OK)
        {
            printf("ERR: Thread Connection to database failed: %s\n", PQerrorMessage(threadPGConn));
            PQfinish(threadPGConn);
            return (void *)1;
        }

        PGresult *pgRes = PQexec(threadPGConn, "BEGIN");

        if (PQresultStatus(pgRes) != PGRES_COMMAND_OK) {

            printf("ERR: BEGIN command failed: %s\n", PQerrorMessage(threadPGConn));
            PQclear(pgRes);
            PQfinish(threadPGConn);
            return (void *)1;
        }
        PQclear(pgRes);

        while(urlToFetch.iso2char)
        {
            //build the url
            //https://itunes.apple.com/WebObjects/MZStoreServices.woa/ws/charts?cc=us&g=36&name=FreeApplications&limit=200

                char *prefix = "https://itunes.apple.com/WebObjects/MZStoreServices.woa/ws/charts?";
            char cc[10] = {0};

            const char *topType = "";
            char *limit = "&limit=100";
            char genre[15] = {0};
            // adding this dummy param-value so that it doesnt hit cache
            char xParam[10] = {0};
            char xValue[15] = {0};

            randomString(5, xParam); //array names are pointers. no need of &xParam
            randomString(10, xValue);

            if(clock_gettime(CLOCK_REALTIME , &startTime))
            {
                printf("ERR : Coudnt get startTime\n");
                startTime.tv_sec = 0;
                startTime.tv_nsec = 0;
            }

            sprintf(cc,"cc=%s",urlToFetch.iso2char);

            topType = storeChartURLSlugs[storeType][urlToFetch.chartType];

            // not checking for zero anymore, as there is no zero
            sprintf(genre,"&g=%i",urlToFetch.genre);

            char *fullUrl;
            //NOTE: cc needs to go first, as it doesnt have the &
            asprintf(&fullUrl,"%s%s%s%s%s&%s=%s", prefix, cc, genre, topType, limit, xParam, xValue);

            //printf("New Url: %s\n",fullUrl);
            if((void *)403 == pull_one_url(curl,fullUrl, urlToFetch.iso2char, urlToFetch.genre , urlToFetch.chartType , threadPGConn))
            {
                printf("ERR : Got 403 , thread will wind up\n");
                //NOTE: "closing a database connection will implicitly rollback any open transaction"
                // got this from stackoverflow comments but cant find it anywhere in documentation (postgres or libpq)
                // IMP: If rollback doesnt happen, the transaction remains and uses resources. Many such transactions can block the db server itself. REVISIT.
                PQfinish(threadPGConn);
                curl_easy_cleanup(curl);
                return (void *)1;
            }

            free(fullUrl); //v imp. bcoz url was created by asprintf

            if(clock_gettime(CLOCK_REALTIME , &endTime))
            {
                printf("ERR : Coudnt get endTime\n");
                endTime.tv_sec = 0;
                endTime.tv_nsec = 0;
            }

            if(endTime.tv_sec - startTime.tv_sec >= 2)
            {
                //this time getting bigger than range of long , so run_nsecs goes -ve
                //and we anyways are skipping sleep for such long times
                //printf("INFO : Skip Sleep 2+\n");
            }
            else
            {
                if ((endTime.tv_nsec - startTime.tv_nsec) < 0)
                {
                    diffTime.tv_sec = endTime.tv_sec - startTime.tv_sec - 1;
                    diffTime.tv_nsec = 1000000000 + endTime.tv_nsec - startTime.tv_nsec;
                }
                else
                {
                    diffTime.tv_sec = endTime.tv_sec - startTime.tv_sec;
                    diffTime.tv_nsec = endTime.tv_nsec - startTime.tv_nsec;
                }

                long run_nsecs = (diffTime.tv_sec * 1000000000) + diffTime.tv_nsec;
                //printf("Run nsec: %ld  :: %ld ms\n", run_nsecs, run_nsecs/1000000);

                struct timespec sleep_t;
                sleep_t.tv_sec = 0;
                sleep_t.tv_nsec = 0;

                if(run_nsecs < oneSec_nsecs)
                {
                    // removed the older switch case logic. that seemed too complex and unecessary. it wasnt adding any randomness too to the execution order of the threads.
                    // they were still firing at same times (when looked at seconds level precision)

                    sleep_t.tv_sec = 1;
                    sleep_t.tv_nsec = halfSec_nsecs + 250000000;  // half second + 250 ms i.e. 750 ms
                }
                else if(run_nsecs < oneAndHalfSec_nsecs)
                {
                    sleep_t.tv_sec = 1;
                    sleep_t.tv_nsec = halfSec_nsecs; //sleep_nsecs;
                }
                else
                {
                    //printf("INFO : Skip Sleep\n");
                    //was skipping sleep here earlier. just sleeping for 250ms
                    sleep_t.tv_sec = 0;
                    sleep_t.tv_nsec = 250000000;  //250 ms
                }
                nanosleep(&sleep_t , NULL);
            }

            urlToFetch = get_one_url_to_fetch();
        }

        pgRes = PQexec(threadPGConn, "COMMIT");

        if (PQresultStatus(pgRes) != PGRES_COMMAND_OK) {

            printf("ERR: COMMIT command failed: %s\n", PQerrorMessage(threadPGConn));
            // following are redundant, so commenting out. Revisit, if flow changes later.
            //PQclear(pgRes);
            //PQfinish(threadPGConn);
            //curl_easy_cleanup(curl);
            //return (void *)1;
        }
        PQclear(pgRes);
        PQfinish(threadPGConn);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headerlist);
        return NULL;
    }

    void printUsage()
    {
        printf("Usage:\n  ts_fetch <store type>\n");
        printf("store type: \n\tiphone\n\tipad\n\tmac\n");
    }

    int main(int argc, char **argv)
    {
        int i;
        int error;
        PGconn *mainPGConn;
        int numThreadsToCreate;

        srand(time(NULL));

        if(argc == 2)
        {
            if(0 == strncmp(argv[1], "iphone", 6))
            {
                printf("Fetching store type: iPhone\n");
                storeType = STORE_TYPE_IPHONE;
            }
            else if(0 == strncmp(argv[1], "ipad", 4))
            {
                printf("Fetching store type: iPad\n");
                storeType = STORE_TYPE_IPAD;
            }
            else if(0 == strncmp(argv[1], "mac", 3))
            {
                printf("Fetching store type: Mac\n");
                storeType = STORE_TYPE_MAC;
            }
            else
            {
                printf("ERR: Unsupported store type!\n");
                printUsage();
                exit(1);
            }
        }
        else
        {
            printf("ERR: No store type provided!\n");
            printUsage();
            exit(1);
        }

        // Fixing no of threads as I have been running with 5 forever.
        numThreadsToCreate = NUM_THREADS;
        pthread_t tid[numThreadsToCreate];

        // libpq tutorials from:
        //   http://zetcode.com/db/postgresqlc/
        //   https://www.postgresql.org/docs/9.6/static/libpq-example.html
        mainPGConn = PQconnectdb("host=localhost port=5432 user=u_rankup password=p_rankup dbname=rankup connect_timeout=30");

        if (PQstatus(mainPGConn) != CONNECTION_OK)
        {
            printf("ERR: Connection to database failed: %s\n", PQerrorMessage(mainPGConn));
            exit_nicely(mainPGConn);
        }

        int ver = PQserverVersion(mainPGConn);

        printf("Connection test success!! Server version: %d\n", ver);
        PQfinish(mainPGConn);

        if(pthread_mutex_init(&allArrayLock , NULL) != 0)
        {
            printf("ERR : mutex init failed!!\n");
            return 1;
        }
        if(build_allobjects_stack())
        {
            printf("ERR : Error in building object stack!! Exiting.\n");
            return 1;
        }

        /* Must initialize libcurl before any threads are started */
        curl_global_init(CURL_GLOBAL_ALL);

        for(i=0; i< numThreadsToCreate; i++)
        {
            /*
                int pthread_create(pthread_t *new_thread_ID,
                                    const pthread_attr_t *attr,
                                    void * (*start_func)(void *),
                                    void *arg);
            */
            error = pthread_create(&tid[i],
                NULL, /* default attributes please */
                startFetching,
                NULL);

            if(0 != error)
                printf("ERR : Couldn't run thread number %d, errno %d\n", i, error);
            else
                printf("INFO : Thread %d, starts\n", i);
        }

        /* now wait for all threads to terminate */
        for(i=0; i< numThreadsToCreate; i++) {
            error = pthread_join(tid[i], NULL);
            printf("INFO : Thread %d terminated\n", i);
        }

        return 0;
    }
