# Airline Checker Simulation Program

Virtual Simulation for Airline check-in with each Economy, Business Queue, and 5 Clerk(modifiable). **Works on Linux, not other OS**

### Key Feature:

- Concurrently progressing by using pthread library.
- Safely handled key data while multi threading.
- Handled prioty of Queues.

### Program Usage:
    $ make
    $ ./ACS input_file.txt

### Program Output:
    output_input_file.txt
    error logs will be printed in output_input_file.txt

### Test Input Files:
    input0.txt -> normal case
    input1.txt -> customer number is different lines of customer
                  handled by adjust customer number.
    input2.txt -> all customer arrives at the same times.
                  case for priority of queues.
    input3.txt -> invalid input of arrival and service time.

### Sample Input:
8
1:0,2,60
2:0,4,70
3:0,5,50
4:1,7,30
5:1,7,40
6:1,8,50
7:0,10,30
8:0,11,50

---------
### Sample Output:
customer arrives: customer ID:  1 
The queue ID  1, and length of the queue  1. 
A clerk starts serving a customer: waited time:0.00 start time 0.20, the customer ID  1, the clerk ID 3. 
customer arrives: customer ID:  2 
The queue ID  1, and length of the queue  1. 
A clerk starts serving a customer: waited time:0.00 start time 0.40, the customer ID  2, the clerk ID 5. 
customer arrives: customer ID:  3 
The queue ID  1, and length of the queue  1. 
A clerk starts serving a customer: waited time:0.00 start time 0.50, the customer ID  3, the clerk ID 2. 
customer arrives: customer ID:  5 
The queue ID  2, and length of the queue  1. 
A clerk starts serving a customer: waited time:0.00 start time 0.70, the customer ID  5, the clerk ID 1. 
customer arrives: customer ID:  4 
The queue ID  2, and length of the queue  1. 
A clerk starts serving a customer: waited time:0.01 start time 0.71, the customer ID  4, the clerk ID 4. 
customer arrives: customer ID:  6 
The queue ID  2, and length of the queue  1. 
customer arrives: customer ID:  7 
The queue ID  1, and length of the queue  1. 
customer arrives: customer ID:  8 
The queue ID  1, and length of the queue  2. 
-->>>A clerk finishes serving a customer: end time 3.71, the customer ID  4, the clerk ID 4. 
A clerk starts serving a customer: waited time:2.91 start time 3.71, the customer ID  6, the clerk ID 4. 
-->>>A clerk finishes serving a customer: end time 4.70, the customer ID  5, the clerk ID 1. 
A clerk starts serving a customer: waited time:3.70 start time 4.70, the customer ID  7, the clerk ID 1. 
-->>>A clerk finishes serving a customer: end time 5.50, the customer ID  3, the clerk ID 2. 
A clerk starts serving a customer: waited time:4.40 start time 5.50, the customer ID  8, the clerk ID 2. 
-->>>A clerk finishes serving a customer: end time 6.20, the customer ID  1, the clerk ID 3. 
-->>>A clerk finishes serving a customer: end time 7.40, the customer ID  2, the clerk ID 5. 
-->>>A clerk finishes serving a customer: end time 7.70, the customer ID  7, the clerk ID 1. 
-->>>A clerk finishes serving a customer: end time 8.71, the customer ID  6, the clerk ID 4. 
-->>>A clerk finishes serving a customer: end time 10.50, the customer ID  8, the clerk ID 2. 
The average waiting time for all customers in queues is: 1.38 sec
Business class customers Avg waited time in queue 0.98 sec
Economy class customers Avg waited time in queue 1.62 sec
