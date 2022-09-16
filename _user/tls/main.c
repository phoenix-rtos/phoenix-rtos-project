// #include <pthread.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <time.h>
// #include <errno.h>
// #include <sys/types.h>
// #include <limits.h>
// #include <assert.h>

// #define THREAD_NUM   50
// #define CHECKS       100
// #define ERRNO_CHECKS 36

// // __thread int tbss;
// // __thread int tdata = 3;

// pthread_mutex_t mutex;

// typedef struct {
// 	int expected_tbss_value;
// 	int expected_tdata_value;
// 	int actual_tbss_value;
// 	int actual_tdata_value;
// 	int *tbss_value_addr;
// 	int *tdata_value_addr;
// 	int passed;
// } tls_check_t;

// typedef struct {
// 	int expected_tls_errno;
// 	int actual_tls_errno;
// 	int *errno_addr;
// 	int passed;
// } tls_errno_check_t;

// void *tls_assign_defaults(void *args)
// {
// 	tls_check_t *res = args;
// 	res->actual_tbss_value = tbss;
// 	res->tbss_value_addr = &tbss;
// 	usleep(100);
// 	res->actual_tdata_value = tdata;
// 	res->tdata_value_addr = &tdata;
// 	return NULL;
// }


// void *tls_change_variables(void *args)
// {
// 	tls_check_t *result = args;
// 	int passed = 0;
// 	for (int i = 0; i < CHECKS; i++) {
// 		result->expected_tbss_value += i;
// 		result->expected_tdata_value += i;
// 		tbss = result->expected_tbss_value;
// 		tdata = result->expected_tdata_value;
// 		result->actual_tbss_value = tbss;
// 		result->actual_tdata_value = tdata;
// 		usleep(100);
// 		if (result->actual_tbss_value == result->expected_tbss_value && result->actual_tdata_value == result->expected_tdata_value) {
// 			passed++;
// 		}
// 	}
// 	result->tdata_value_addr = &tdata;
// 	result->tbss_value_addr = &tbss;
// 	result->passed = passed;

// 	return NULL;
// }


// void *tls_check_errno(void *args)
// {
// 	tls_errno_check_t *result = args;
// 	int passed = 0;
// 	for (int i = 0; i < ERRNO_CHECKS; i++) {
// 		SET_ERRNO(result->expected_tls_errno + i);
// 		usleep(100);
// 		result->actual_tls_errno = -errno;
// 		if (result->actual_tls_errno == result->expected_tls_errno + i) {
// 			passed++;
// 		}
// 	}
// 	result->errno_addr = &errno;
// 	result->passed = passed;
// 	return NULL;
// }



// void *doSth2(void *args)
// {
// 	pthread_mutex_lock(&mutex);
// 	printf("%d\n", tbss);
// 	printf("%d\n", tdata);
// 	pthread_mutex_unlock(&mutex);
// 	return NULL;
// }

// int maix(int argc, char const *argv[])
// {
// 	pthread_mutex_init(&mutex, NULL);
// 	pthread_t threads[THREAD_NUM];
// 	for (int i = 0; i < THREAD_NUM; i++)
// 	{
// 		pthread_create(&threads[i], NULL, doSth2, NULL);
// 	}
// 	for (int i = 0; i < THREAD_NUM; i++)
// 	{
// 		pthread_join(threads[i], NULL);
// 	}
	
// }

// int main(int argc, char const *argv[])
// {
// 	tls_check_t results[THREAD_NUM + 1];
// 	pthread_t threads[THREAD_NUM];
// 	printf("%p\n", &tbss);
// 	printf("%p\n", &tdata);
// 	printf("%p\n", &errno);
// 	srand(time(NULL));

// 	results[THREAD_NUM].expected_tbss_value = rand() % (INT_MAX / 2);
// 	results[THREAD_NUM].expected_tdata_value = rand() % (INT_MAX / 2);
// 	tls_change_variables(&results[THREAD_NUM]);

// 	for (int i = 0; i < THREAD_NUM; i++) {
// 		results[i].expected_tbss_value = rand() % (INT_MAX / 2);
// 		results[i].expected_tdata_value = rand() % (INT_MAX / 2);
// 		pthread_create(&threads[i], NULL, tls_change_variables, &results[i]);
// 	}
// 	for (int i = 0; i < THREAD_NUM; i++) {
// 		pthread_join(threads[i], NULL);
// 	}
// 	for (int i = 0; i < THREAD_NUM + 1; i++) {
// 		assert(CHECKS == results[i].passed);
// 		for (int j = i + 1; j < THREAD_NUM + 1; j++) {
// 			assert(results[i].tbss_value_addr != results[j].tbss_value_addr);
// 			assert(results[i].tdata_value_addr != results[j].tdata_value_addr);
// 			assert(results[i].tbss_value_addr != results[i].tdata_value_addr);
// 		}
// 		printf("%d\n", results[i].actual_tbss_value);
// 	}
// 	return 0;
// }
