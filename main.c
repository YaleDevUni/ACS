#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
// global variable
pthread_mutex_t b_mutex;
pthread_mutex_t e_mutex;
pthread_cond_t b_cond;
pthread_cond_t e_cond;
int clerk_status[5] = {0, 0, 0, 0, 0}; //0 indicate it's idle
int queue_length[2] = {0, 0};
int queue_status[2] = {0, 0};
int winner_selected[2] = {0, 0};
int NUM_CLERK = 5;
int NUM_CUST;
int equeue[1000];
int bqueue[1000];
float b_waiting_time=0;
float e_waiting_time=0;
FILE * outfile;


static struct timeval start_time;
typedef struct customer_info
{
    int user_id;
    int class_type;
    int service_time;
    int arrival_time;
} customer_info;
customer_info *customers;

customer_info *read_file(char *file_name, int *r_cst_num);
double getCurrentTime();
void enQueue(int cust_class, int cust_ID);
int getHead(int cust_class);
int deQueue(int cust_class);

void *customer_entry(void *cus_info)
{
    //get customer info
    struct customer_info p_myInfo = *(struct customer_info *)cus_info;
    int user_id = p_myInfo.user_id;
    int cust_class = p_myInfo.class_type;
    int m_clerk = -1;
    //get time for stats of wating time and UI
    float enter_queue_time=0;
    float service_start_time =0;
    float waited_time =0;
    gettimeofday(&start_time, NULL);
    //wait for arrival time
    usleep(p_myInfo.arrival_time * 100000);
    fprintf(outfile, "customer arrives: customer ID: %2d \n", user_id);
    //case for business class customer
    if (cust_class == 1)
    {
        //protect data
        pthread_mutex_lock(&b_mutex);
        // customer enter the queue
        enQueue(1, user_id);
        queue_length[1]++;
        fprintf(outfile, "The queue ID %2d, and length of the queue %2d. \n", cust_class+1, queue_length[cust_class]);
        enter_queue_time=getCurrentTime();
        while (1)
        {
            //customers wake up everytime clerk signalling
            pthread_cond_wait(&b_cond, &b_mutex);
            int head_of_line = getHead(1);
            // wait until customer is head of queue
            if ((user_id == head_of_line) && (winner_selected[1] == 0))
            {
                // get clerk id who serve customer
                m_clerk = queue_status[1];
                //check clerk is idle or not
                if(clerk_status[m_clerk]!=1){
                    // set clerk as busy
                    clerk_status[m_clerk] = 1;
                    //dequeue the queue
                    deQueue(1);
                    queue_length[1]--;
                    // lock the door to prevent customers exit the queue at same time
                    winner_selected[1] = 1;
                    //get time for stats
                    service_start_time = getCurrentTime();
                    waited_time = service_start_time-enter_queue_time;
                    b_waiting_time = b_waiting_time+waited_time;
                    break;
                }
            }
        }
        queue_status[1] = 0;
        // unlock the data
        pthread_mutex_unlock(&b_mutex);
        usleep(1);
        fprintf(outfile, "A clerk starts serving a customer: waited time:%.2f start time %.2f, the customer ID %2d, the clerk ID %1d. \n",waited_time, getCurrentTime(), user_id, m_clerk);
        //customer served by clerk
        usleep(p_myInfo.service_time * 100000);
        fprintf(outfile, "-->>>A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d. \n", getCurrentTime(), user_id, m_clerk);
        //set clerk status as idle
        clerk_status[m_clerk] = 0;
    }
    else//for economic class. Same process above codes
    {
        pthread_mutex_lock(&e_mutex);
        enQueue(0, user_id);
        queue_length[0]++;
        fprintf(outfile, "The queue ID %2d, and length of the queue %2d. \n", cust_class+1, queue_length[cust_class]);
        enter_queue_time=getCurrentTime();
        while (1)
        {
            pthread_cond_wait(&e_cond, &e_mutex);
            int head_of_line = getHead(0);
            if ((user_id == head_of_line) && (winner_selected[0] == 0))
            {
                m_clerk = queue_status[0];
                if(clerk_status[m_clerk]!=1){
                clerk_status[m_clerk] = 1;
                deQueue(0);
                queue_length[0]--;
                winner_selected[0] = 1;
                service_start_time = getCurrentTime();
                waited_time = service_start_time-enter_queue_time;
                e_waiting_time=e_waiting_time+waited_time;
                break;
                }
            }
        }
        queue_status[0] = 0;
        pthread_mutex_unlock(&e_mutex);
        usleep(10);
        fprintf(outfile, "A clerk starts serving a customer: waited time:%.2f start time %.2f, the customer ID %2d, the clerk ID %1d. \n",waited_time, getCurrentTime(), user_id, m_clerk);
        usleep(p_myInfo.service_time * 100000);
        fprintf(outfile, "-->>>A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d. \n", getCurrentTime(), user_id, m_clerk);
        clerk_status[m_clerk] = 0;
    }

    pthread_exit(NULL);
    // fprintf(stdout, "A clerk starts serving a customer: start time %.2f, the customer ID %2d, the clerk ID %1d. \n");
    return NULL;
}
void *clerk_entry(void *clerk_ID)
{
    int clerkID = *(int *)clerk_ID;
    while (1)
    {
        //give higher priority to business queue
        //if business queue is not empty call business queue first
        if (queue_length[1] > 0 )
        {
            //protect business queue data
            pthread_mutex_lock(&b_mutex);
            //send clerk id to customer
            queue_status[1] = clerkID;
            //wake up customers
            pthread_cond_broadcast(&b_cond);
            //open door 
            winner_selected[1] = 0;
            //release mutex
            pthread_mutex_unlock(&b_mutex);
            //while clerk is busy 
            while (clerk_status[clerkID] != 0)
            {
            }
        }
        else if(queue_length[1]==0)
        {
            pthread_mutex_lock(&e_mutex);
            queue_status[0] = clerkID;
            pthread_cond_broadcast(&e_cond);
            winner_selected[0] = 0;
            pthread_mutex_unlock(&e_mutex);
            while (clerk_status[clerkID] != 0)
            {
            }
        }
    }
    pthread_exit(NULL);
    return (NULL);
}

int main(int argc, char *argv[])
{
    if(argc!=2){
        printf("Invalid command.... Usage: ./ACS [input_file.txt]\n");
        exit(-1);
    }
    // gettimeofday(&start_time, NULL);
    char dst_file_name[100] = "output_";
    strcat(dst_file_name,argv[1]);
    outfile = fopen(dst_file_name,"w");
    int num_bcust =0;
    int num_ecust =0;
    customers = read_file(argv[1], &NUM_CUST);
    pthread_t customer_thread[NUM_CUST];
    pthread_t clerk_thread[NUM_CLERK];
    // init mutexes and condtion variables
    pthread_mutex_init(&b_mutex, NULL);
    pthread_mutex_init(&e_mutex, NULL);
    pthread_cond_init(&b_cond, NULL);
    pthread_cond_init(&e_cond, NULL);
    // create threads
    for (int i = 0; i < NUM_CLERK; i++)
    {
        int *clerk_ID = malloc(sizeof(int));
        *clerk_ID = i + 1;
        if (pthread_create(&clerk_thread[i], NULL, clerk_entry, clerk_ID) != 0)
        {
            perror("Fail to create thread");
        }
    }
    for (int i = 0; i < NUM_CUST; i++)
    {
        if((customers[i].class_type) == 0){
            num_ecust++;
        }else{
            num_bcust++;
        }
        struct customer_info *cust_info = malloc(sizeof(*cust_info));
        *cust_info = customers[i];

        if (pthread_create(&customer_thread[i], NULL, customer_entry, cust_info) != 0)
        {
            perror("Fail to create thread");
        }
    }
    // wait for customer thread all terminated
    for (int i = 0; i < NUM_CUST; i++)
    {
        if (pthread_join(customer_thread[i], NULL) != 0)
        {
            perror("Failed to join thread");
        }
    }
    // print stats
    fprintf(outfile,"The average waiting time for all customers in queues is: %.2f sec\n",(b_waiting_time+e_waiting_time)/NUM_CUST);
    fprintf(outfile,"Business class customers Avg waited time in queue %.2f sec\n",b_waiting_time/num_bcust);
    fprintf(outfile,"Economy class customers Avg waited time in queue %.2f sec\n",e_waiting_time/num_ecust);
    fclose(outfile);
    return (0);
}

customer_info *read_file(char *file_name, int *r_cst_num)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    fp = fopen(file_name, "r");
    if (fp == NULL){
        printf("%s doesn't exist....\n",file_name);
        exit(EXIT_FAILURE);
    }
    int cst_num = 0;
    // get customer number
    if (getline(&line, &len, fp) != -1)
    {
        cst_num = atoi(line);
        if(cst_num==0){
            printf("no customers... program finished..\n");
            exit(0);
        }
    }
    // update customer number of outside of function
    *r_cst_num = cst_num;
    int cnt = 0;
    // init customer info array
    customer_info *cst_array = (malloc(cst_num * sizeof(*cst_array)));
    // read required inforamtion from input file
    int negative_dectected = 0;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        //error handler for number customer input format invalid
        if(cnt==cst_num){
            fprintf(outfile,"*---------------Error Log----------------------------------*\n");
            fprintf(outfile,"input customer number and lines of customer doesn't match\n");
            fprintf(outfile,"Customer number Fixed, Customer number:%d\n",cnt);
            fprintf(outfile,"*---------------Error Log----------------------------------*\n");
            *r_cst_num = cst_num;
        break;
        }
        char *pChr = strtok(line, ":,");
        for (int i = 0; i < 4; i++)
        {
            switch (i)
            {
            case 0:
                cst_array[cnt].user_id = atoi(pChr);
                break;
            case 1:
                cst_array[cnt].class_type = atoi(pChr);
                break;
            case 2:
                if(atoi(pChr)<0){
                    fprintf(outfile,"negative aririval time detected on customer ID:%d\n",cnt+1);
                    negative_dectected++;
                }
                cst_array[cnt].arrival_time = atoi(pChr);
                break;
            case 3:
                if(atoi(pChr)<0){
                    fprintf(outfile,"negative service time detected on customer ID:%d\n",cnt+1);
                    negative_dectected++;
                }
                cst_array[cnt].service_time = atoi(pChr);
                break;
            }
            pChr = strtok(NULL, ":,");
        }

        cnt++;

        
    }
    //error handler for number customer input format invalid
    if(cnt!=cst_num){
            fprintf(outfile,"*---------------Error Log----------------------------------*\n");
            fprintf(outfile,"input customer number and lines of customer doesn't match\n");
            fprintf(outfile,"Customer number Fixed, Customer number:%d\n",cnt);
            fprintf(outfile,"*---------------Error Log----------------------------------*\n");
            *r_cst_num = cnt;
    }
    //error handler for negative int value in input file
    if(negative_dectected>0){
        printf("invalid value or format detected in input file. To see log of errer, check ouput_%s\n",file_name);
        fclose(fp);
        fclose(outfile);
        exit(-1);
    }
    fclose(fp);
    if (line)
        free(line);
    return cst_array;
}
// get current time in floating type
double getCurrentTime()
{
    struct timeval cur_time;
    double cur_secs, init_secs;

    //pthread_mutex_lock(&start_time_mtex); you may need a lock here
    init_secs = (start_time.tv_sec + (double)start_time.tv_usec / 1000000);
    //pthread_mutex_unlock(&start_time_mtex);

    gettimeofday(&cur_time, NULL);
    cur_secs = (cur_time.tv_sec + (double)cur_time.tv_usec / 1000000);

    return cur_secs - init_secs;
}
// enqueue selected queue
void enQueue(int cust_class, int cust_ID)
{
    /* determine customer class*/
    if (cust_class == 0)
    {
        equeue[queue_length[0]] = cust_ID;
    }
    else
    {
        bqueue[queue_length[1]] = cust_ID;
    }

}
//get head of selcted queue
int getHead(int cust_class)
{
    if (cust_class == 0)
    {
        return equeue[0];
    }
    else
    {
        return bqueue[0];
    }
}
//dequeue selected queue
int deQueue(int cust_class)
{
    int res;
    if (cust_class == 0)
    {
        if (queue_length[0] == 0)
        {
            fprintf(stderr, "No element to deque....\n");
            return -1;
        }
        res = equeue[0];
        for (int i = 0; i < queue_length[0] - 1; i++)
        {
            equeue[i] = equeue[i + 1];
        }
    }
    else
    {
        if (queue_length[1] == 0)
        {
            fprintf(stderr, "No element to deque....\n");
            return -1;
        }
        res = bqueue[0];
        for (int i = 0; i < queue_length[1] - 1; i++)
        {
            bqueue[i] = bqueue[i + 1];
        }
    }
    return res;
}
