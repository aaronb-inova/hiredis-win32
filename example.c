#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hiredis.h"

#ifdef HIREDIS_WIN
#define snprintf sprintf_s
#endif

const int PORT = 6379;
const char *const AUTH_CMD = NULL;

int main(void) {
    unsigned int j;
    redisContext *c;
    redisReply *reply;
    int result;

    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    c = redisConnectWithTimeout((char*)"127.0.0.1", PORT, timeout);
    if (c == NULL || c->err) {
        if (c) {
        printf("Connection error: %s\n", c->errstr);
            redisFree(c);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }

    /* AUTH server */
    if( NULL != AUTH_CMD )
    {
        reply = (redisReply *)redisCommand(c, AUTH_CMD);
        printf("AUTH: %s\n", reply->str);
        freeReplyObject(reply);
    }


    /* PING server */
    reply = (redisReply *)redisCommand(c,"PING");
    printf("PING: %s\n", reply->str);
    freeReplyObject(reply);

    /* Set a key */
    reply = (redisReply *)redisCommand(c,"SET %s %s", "foo", "hello world");
    printf("SET: %s\n", reply->str);
    freeReplyObject(reply);

    /* Set a key using binary safe API */
    reply = (redisReply *)redisCommand(c,"SET %b %b", "bar", 3, "hello", 5);
    printf("SET (binary API): %s\n", reply->str);
    freeReplyObject(reply);

    /* Try a GET and two INCR */
    reply = (redisReply *)redisCommand(c,"GET foo");
    printf("GET foo: %s\n", reply->str);
    freeReplyObject(reply);

    /* pipeline a del and two increments again ... */
    result = redisAppendCommand(c,"DEL counter");
    if ( REDIS_OK != result ) { printf("Got an error from DEL counter: %d\n", result); }
    redisAppendCommand(c,"INCR counter");
    if ( REDIS_OK != result ) { printf("Got an error from first INCR counter: %d\n", result); }
    redisAppendCommand(c,"INCR counter");
    if ( REDIS_OK != result ) { printf("Got an error from second INCR counter: %d\n", result); }
    result = redisGetReply(c, (void **)&reply);
    if( REDIS_OK == result ) { freeReplyObject(reply); }
    result = redisGetReply(c, (void **)&reply);
    if( REDIS_OK == result ) 
    { 
        printf("INCR counter: %lld\n", reply->integer);
        freeReplyObject(reply); 
    }
    result = redisGetReply(c, (void **)&reply);
    if( REDIS_OK == result ) 
    { 
        printf("INCR counter: %lld\n", reply->integer);
        freeReplyObject(reply); 
    }

    /* Create a list of numbers, from 0 to 9 */
    reply = (redisReply *)redisCommand(c,"DEL mylist");
    freeReplyObject(reply);
    for (j = 0; j < 10; j++) {
        char buf[64];

        snprintf(buf,64,"%d",j);
        reply = (redisReply *)redisCommand(c,"LPUSH mylist element-%s", buf);
        freeReplyObject(reply);
    }

    /* Let's check what we have inside the list */
    reply = (redisReply *)redisCommand(c,"LRANGE mylist 0 -1");
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (j = 0; j < reply->elements; j++) {
            printf("%u) %s\n", j, reply->element[j]->str);
        }
    }
    freeReplyObject(reply);

    return 0;
}
