#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXLINE 100

int countLine(char *filename);
int * getAddresses(int *startAddresses);
int * getLengthArr(int *readLengthArr);
void *reader(void* arg);
void *replace(void* arg);
void *upper(void* arg);
void *writeToFile(void* arg);
int findCommandLength(char * command);
int arrangeWriteLines();

typedef struct node{  //LINE NODE
    char line[MAXLINE];               //storing the line inside here
    int lengthOfLine;
    int readLineIndex;                   //Index of line
    int controlRead;
    int uppered;                 //For upper control
    int replaced;               //For replace control
    pthread_mutex_t key;        //Mutex for each line
    pthread_mutex_t writeKey;
    int writed;
    struct node *next;
}Node;

//reader() arguments
typedef struct{
    pthread_t tid;
    int readLineIndex;          //Index of line
    int startAddress;
    int readLength;
    int threadId;
    int readOnce;
    int * startAddresses ;
    int * readLengthArr;
    Node * Line; // read thread arguments
} read_runner;

//REPLACE ARGS
typedef struct {
    pthread_t tid;
    int replaceLineIndex;          //Index of line
    int replacedOnce;
}replace_runner;


//UPPER ARGS
typedef struct {
    pthread_t tid;
    int upperLineIndex;          //Index of line
    int upperedOnce;
}upper_runner;


//UPPER ARGS
typedef struct {
    pthread_t tid;
    int writeLineIndex;          //Index of line
    int wroteOnce;
    int * startAddresses ;
    int * readLengthArr;
}write_runner;

int numberOfLine;
int read_t_number = 0;
int replace_t_number = 0;
int upper_t_number = 0;
int write_t_number = 0;


Node * Lines [100];
Node * unchangedLines [100];
pthread_mutex_t readCounterKey = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t replaceCounterKey = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t upperCounterKey = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t writeCounterKey = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t writeToFileKey = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t lineKeys [100];
pthread_mutex_t writeKeys [100];

int readLineCount = 0;
int replaceLineCount = 0;
int upperLineCount = 0;
int writeLineCount = 0;
char fileName[100];
int beenRead [100];

int main(int argc,char* argv[]) {

    if(argc == 1)
        exit(0);

    strcpy(fileName, argv[2]);

    printf("Filename: %s\n", fileName);

    numberOfLine = countLine(fileName);

    for (int k = 0; k < numberOfLine; k++) {
        pthread_mutex_init(&(lineKeys[k]), NULL);
        pthread_mutex_lock(&(lineKeys[k]));
    }

    for (int k = 0; k < numberOfLine; k++) {
        pthread_mutex_init(&(writeKeys[k]), NULL);
        pthread_mutex_lock(&(writeKeys[k]));
    }

    int startAddressesTemp[numberOfLine];
    int readLengthArrTemp[numberOfLine];

    int * startAddresses = getAddresses(startAddressesTemp);
    int * readLengthArr = getLengthArr(readLengthArrTemp);
    int * startAddresses2 = getAddresses(startAddressesTemp);
    int * readLengthArr2 = getLengthArr(readLengthArrTemp);


    read_t_number = atoi(argv[4]);
    pthread_t read_threads[read_t_number];
    read_runner read_thread_args [read_t_number]; // read thread arguments
    readLineCount = read_t_number;

    replace_t_number = atoi(argv[6]);
    pthread_t replace_threads[replace_t_number];
    replace_runner replace_thread_args [replace_t_number]; // replace thread arguments
    replaceLineCount = replace_t_number;

    upper_t_number = atoi(argv[5]);
    pthread_t upper_threads[upper_t_number];
    upper_runner upper_thread_args [upper_t_number]; // upper thread arguments
    upperLineCount = upper_t_number;

    write_t_number = atoi(argv[7]);
    pthread_t write_threads[write_t_number];
    write_runner write_thread_args [write_t_number]; // upper thread arguments
    writeLineCount = write_t_number;

    for(int i = 0; i < read_t_number; i++ ) {

        read_thread_args[i].readLineIndex = i;
        read_thread_args[i].startAddress = startAddresses[i]; //start address of each thread to read
        read_thread_args[i].readLength = readLengthArr[i];   //length of string will be read of each thread
        read_thread_args[i].threadId = i;
        read_thread_args[i].readLengthArr= readLengthArr;
        read_thread_args[i].startAddresses= startAddresses;
        read_thread_args[i].readOnce= 0;
    }

    for(int i = 0; i < replace_t_number; i++ ) {
        replace_thread_args[i].tid = i;
        replace_thread_args[i].replaceLineIndex = i;
        replace_thread_args[i].replacedOnce = 0;
    }

    for(int i = 0; i < upper_t_number; i++ ) {
        upper_thread_args[i].tid = i;
        upper_thread_args[i].upperLineIndex = i;
        upper_thread_args[i].upperedOnce = 0;
    }

    for(int i = 0; i < write_t_number; i++ ) {
        write_thread_args[i].tid = i;
        write_thread_args[i].writeLineIndex = i;
        write_thread_args[i].readLengthArr= readLengthArr2;
        write_thread_args[i].startAddresses= startAddresses2;
        write_thread_args[i].wroteOnce= 0;
    }

    printf("<Thread-type and ID>\t<Output>\n");

    //Create read threads
    for(int i = 0; i < read_t_number; i++) {
        int chck = pthread_create(&read_threads[i], NULL, &reader , &read_thread_args[i]);
        if(chck){ printf("Read thread couldn't creat"); break; }
    }
    fflush(stdout);

    //Create replace threads
    for(int i = 0; i < replace_t_number; i++) {
        int chck = pthread_create(&replace_threads[i], NULL, &replace , &replace_thread_args[i]);
        if(chck){ printf("Replace thread couldn't creat"); break; }
    }
    fflush(stdout);


    //Create upper threads
    for(int i = 0; i < upper_t_number; i++) {
        int chck = pthread_create(&upper_threads[i], NULL, &upper , &upper_thread_args[i]);
        if(chck){ printf("Upper thread couldn't creat"); break; }
    }
    fflush(stdout);

    //Create write threads
    for(int i = 0; i < write_t_number; i++) {
        int chck = pthread_create(&write_threads[i], NULL, &writeToFile , &write_thread_args[i]);
        if(chck){ printf("Write thread couldn't creat"); break; }
    }
    fflush(stdout);

    //Joins
    for(int i = 0; i < read_t_number; i++){
        pthread_join(read_threads[i], NULL);
    }

    //Joins
    for(int i = 0; i < replace_t_number; i++) {
        pthread_join(replace_threads[i], NULL);
    }

    //Joins
    for(int i = 0; i < upper_t_number; i++){
        pthread_join(upper_threads[i], NULL);
    }


    //Joins
    for(int i = 0; i < write_t_number; i++){
        pthread_join(write_threads[i], NULL);
    }

}

void *reader(void* arg) {

    read_runner * st = (read_runner*) arg;

    int * addresses =st->startAddresses;
    int * lengths =st->readLengthArr;

    while (1){

        int readCount = 0;

        FILE *fp = fopen(fileName, "r");
        char temp[MAXLINE];

        if(st->readOnce == 0){
            fseek(fp, st->startAddress, SEEK_SET);
            fread(temp, st->readLength, 1, fp);

        }
        else{
            if (pthread_mutex_lock (&readCounterKey) == 0) {
                readLineCount++;
                readCount = readLineCount;
                st->readLineIndex = readCount-1;

                if(readCount > numberOfLine){
                    pthread_mutex_unlock(&readCounterKey);
                    break;
                }

                pthread_mutex_unlock(&readCounterKey);
            }

            fseek(fp, addresses[readCount-1], SEEK_SET);
            fread(temp, lengths[readCount-1], 1, fp);
        }

        Node * tempnode = (Node *) malloc(sizeof(Node));
        strcpy(tempnode->line, temp);
        tempnode->lengthOfLine = findCommandLength(temp);
        beenRead[st->readLineIndex] =1;

        printf("Read_%d\t\t\t Read_%d read the line%d which is \"%s\"\n", st->threadId, st->threadId, (st->readLineIndex)+1, tempnode->line);
        //printf("\nREAD LINE: %s ID: %d\n", tempnode->line, st->threadId);

        fflush(stdout);

        Lines[st->readLineIndex] = tempnode;

        Node * tempnode2 = (Node *) malloc(sizeof(Node));
        strcpy(tempnode2->line, temp);
        unchangedLines[st->readLineIndex] = tempnode2;

        fclose(fp);
        st->readOnce = 1;

        pthread_mutex_init(&(tempnode->writeKey), NULL);

        if (pthread_mutex_lock (&(tempnode->writeKey)) == 0){
            fflush(stdout);
        }

        pthread_mutex_unlock(&(lineKeys[st->readLineIndex]));

    }

    pthread_exit(0);
}

void *replace(void* arg){

    replace_runner *st = (replace_runner *) arg;

    while(1){

        if(st->replacedOnce == 1 ){

            if (pthread_mutex_lock (&replaceCounterKey) == 0) {
                replaceLineCount++;
                st->replaceLineIndex =replaceLineCount-1 ;


                if(replaceLineCount > numberOfLine){
                    pthread_mutex_unlock(&replaceCounterKey);
                    break;
                }
                pthread_mutex_unlock(&replaceCounterKey);
            }
        }

        if (pthread_mutex_lock(&(lineKeys[st->replaceLineIndex])) == 0) {

            /* Success!  This thread now owns the lock. */
            int length = findCommandLength(Lines[st->replaceLineIndex]->line);

            char temp [length];
            strcpy(temp, Lines[st->replaceLineIndex]->line);

            for (int i = 0; i < length; i++) {
                char c = temp[i];
                if(c == ' ')
                    temp[i] = '_';
            }

            strcpy((char *) Lines[st->replaceLineIndex], temp);

            printf("Replace_%d\t\t Replace_%d read index %d and converted \"%s\" to \"%s\"\n", st->tid, st->tid, (st->replaceLineIndex)+1, unchangedLines[st->replaceLineIndex]->line ,Lines[st->replaceLineIndex]->line);

            //printf("\nReplaced line: %s - REPLACE id: %ld \n", Lines[st->replaceLineIndex]->line, st->tid);
            fflush(stdout);

            Lines[st->replaceLineIndex]->replaced = 1;
            //go to write queue
            if(Lines[st->replaceLineIndex]->uppered == 1)
                pthread_mutex_unlock(&(writeKeys[st->replaceLineIndex]));
            pthread_mutex_unlock(&(lineKeys[st->replaceLineIndex]));
        }
        else {
            /* Fail!  This thread doesn't own the lock.  Do something else... */
            printf("REPLACE does not have lock\n");
            fflush(stdout);
        }
        st->replacedOnce = 1;
    }
    pthread_exit(0);
}

void *upper(void* arg) {
    upper_runner *st = (upper_runner *) arg;

    while(1){

        if(st->upperedOnce == 1 ){

            if (pthread_mutex_lock (&upperCounterKey) == 0) {
                upperLineCount++;
                st->upperLineIndex =upperLineCount-1 ;

                if(upperLineCount > numberOfLine){
                    pthread_mutex_unlock(&upperCounterKey);
                    break;
                }
                pthread_mutex_unlock(&upperCounterKey);
            }
        }

        if (pthread_mutex_lock(&(lineKeys[st->upperLineIndex])) == 0) {

            /* Success!  This thread now owns the lock. */
            int length = findCommandLength(Lines[st->upperLineIndex]->line);

            char temp[length];
            strcpy(temp, Lines[st->upperLineIndex]->line);

            for (int i = 0; i < length; i++) {
                char c = temp[i];
                temp[i] = toupper(c);
            }

            strcpy(Lines[st->upperLineIndex], temp);

            printf("Upper_%d\t\t\t Upper_%d read index %d and converted to \"%s\"\n", st->tid, st->tid,
                   (st->upperLineIndex) + 1,  Lines[st->upperLineIndex]->line);

            //printf("Upper_%d\t\t\t\t %s\n", st->tid, Lines[st->upperLineIndex]->line);

            //printf("\nUppered line: %s - UPPER id: %ld \n", Lines[st->upperLineIndex]->line, st->tid);
            fflush(stdout);

            Lines[st->upperLineIndex]->uppered = 1;

            //go to write queue
            if (Lines[st->upperLineIndex]->replaced == 1)
                pthread_mutex_unlock(&(writeKeys[st->upperLineIndex]));

            pthread_mutex_unlock(&(lineKeys[st->upperLineIndex]));
        }
        else {
            /* Fail!  This thread doesn't own the lock.  Do something else... */
            printf("UPPER does not have lock\n");
            fflush(stdout);
        }
        st->upperedOnce = 1;
    }
    pthread_exit(0);
}

void *writeToFile(void* arg) {

    write_runner *st = (write_runner *) arg;

    int * addresses =st->startAddresses;
    int * lengths =st->readLengthArr;

    while(1){

        if(st->wroteOnce == 1 ){

            if (pthread_mutex_lock (&(writeCounterKey)) == 0) {

                writeLineCount++;
                st->writeLineIndex = writeLineCount-1 ;

                if(writeLineCount > numberOfLine){
                    pthread_mutex_unlock(&writeCounterKey);
                    break;
                }
                pthread_mutex_unlock(&(writeCounterKey));
            }
        }

        if (beenRead[st->writeLineIndex] == 1 && pthread_mutex_lock(&(writeKeys[st->writeLineIndex])) == 0){

            //printf("WRITED: %s write tid : %d\n",  Lines[st->writeLineIndex]->line, st->tid);

            printf("Writer_%d\t\t Writer_%d write line %d back which is \"%s\"\n", st->tid, st->tid, (st->writeLineIndex)+1 ,Lines[st->writeLineIndex]->line);

            //printf("Writer_%d\t\t\t %s\n", st->tid, Lines[st->writeLineIndex]->line);
            fflush(stdout);

            Lines[st->writeLineIndex]->writed = 1;

            FILE *fp;
            int address = addresses[st->writeLineIndex];
            int length = lengths[st->writeLineIndex];
            char temp[length];

            strcpy(temp,Lines[st->writeLineIndex]->line);

            int arrange = arrangeWriteLines();

            if(arrange == 1){

                fp = fopen(fileName, "w");

                for (int i = 0; i < numberOfLine; i++) {
                    strcpy(temp,Lines[i]->line);
                    fputs(temp, fp);
                    fputc('\n',fp);
                }
                fclose(fp);

            }

        }
        st->wroteOnce = 1;
    }
    pthread_exit(0);
}

int arrangeWriteLines(){

    int countWritedLines = 0;
    for (int i = 0; i < numberOfLine; i++) {
        if(Lines[i]->writed == 1){
            countWritedLines++;
            if(countWritedLines == numberOfLine)
                return 1;
        }
    }
    return 0;
}

int countLine(char *filename){
    FILE *fp;
    int countLine = 0;  // Line counter (result)
    char c;  // To store a character read from file

    // Open the file
    fp = fopen(filename, "r");

    // Check if file exists
    if (fp == NULL)
    {
        printf("Could not open file %s", filename);
        exit(0);
    }

    // Extract characters from file and store in character c
    for (c = getc(fp); c != EOF; c = getc(fp)){
        if (c == '\n') // Increment count if this character is newline
            countLine = countLine + 1;
    }

    // Close the file
    fclose(fp);
    // printf("The file %s has %d lines\n ", filename, countLine);
    return countLine;
}

int * getAddresses(int *startAddresses){

    FILE *fp = fopen(fileName, "r");
    int lineCount = countLine(fileName);
    int startAddressOfThread = 0;
    //  int startAddresses[lineCount];

    for (int i = 0; i < lineCount; i++){
        int c, count;
        count = 0;
        while(1){
            c = fgetc( fp );
            if( c == EOF || c == '\n' )
                break;
            ++count;
        }
        //printf("line length: %d\n", count);
        startAddresses[i] = startAddressOfThread;
        startAddressOfThread += count + 1;
    }

    fclose(fp);
    return startAddresses;
}

int * getLengthArr(int *readLengthArr){
    FILE *fp = fopen(fileName, "r");
    int lineCount = countLine(fileName);
    // int readLengthArr[lineCount];

    for (int i = 0; i < lineCount; i++){
        int c, count;
        count = 0;
        while(1){
            c = fgetc( fp );
            if( c == EOF || c == '\n' )
                break;
            ++count;
        }
        //printf("line length: %d\n", count);
        readLengthArr[i] = count;
    }

    fclose(fp);
    return readLengthArr;
}

int findCommandLength(char * command){
    int num= 0;
    int spacenum=0;
    for (int j = 0; j < MAXLINE; j++) {  //define length of history buffer
        if(command[j] == '\0'){
            break;
        }
        if(command[j] == ' ')
            spacenum++;

        num++;
    }
    if(spacenum == 0)
        num--;
    return num;
}
