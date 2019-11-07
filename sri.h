#ifndef SRI_H_ //verhindert, dass ein header zweimal eingebunden wird
#define SRI_H_ //da man nur einmal deklarieren darf

#include <pthread.h>

typedef struct {
	int target_phase;
	int threshold;
	pthread_mutex_t mutx; //is in state locked or unlocked
} condition;

typedef struct {
	condition primary;
	condition secondary;
} cond_pair;



int compare_conditions(condition, condition);

extern condition standard;



#endif /* UTIL_H_ */
