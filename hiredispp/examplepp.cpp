#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "hiredispp.h"

int main(int argc, const char *argv[]) 
{
    std::string auth;
    int port = 6397;
    if(argc > 1)
        auth = argv[1];
    if(argc > 2 )
        port = atoi(argv[2]);

    try
    {
        printf("using: localhost:%d, and %s\n", port, auth.empty() ? "NO auth" : "AUTH enabled");
        hiredispp::Redis redis("localhost", auth, port);

        printf("ping: %s\n", redis.ping().c_str());

        std::map<std::string, std::string> info = redis.info();
        printf("info:\n");
        for( std::map<std::string, std::string>::const_iterator i = info.begin(); i != info.end(); ++i )
        {
            printf("    %s: %s\n", i->first.c_str(), i->second.c_str());
        }
    
        redis.set("good", "food");
        printf("get: %s\n", redis.get("good").c_str());

        redis.beginDel("counter");
        redis.beginIncr("counter");
        redis.beginIncr("counter");
        redis.endCommand();
        printf("incr 1: %ld\n", (long)redis.endCommand());
        printf("incr 2: %ld\n", (long)redis.endCommand());
    }
    catch(hiredispp::RedisException e)
    {
        printf("Caught exception: %s\n", e.what());
    }

    return 0;
}
