#include <inc/assert.h>

#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>


// Choose a user environment to run and run it.
void
sched_yield(void)
{
	struct Env *idle;
	int i;

	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running.  Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING) and never choose an
	// idle environment (env_type == ENV_TYPE_IDLE).  If there are
	// no runnable environments, simply drop through to the code
	// below to switch to this CPU's idle environment.

	// LAB 4: Your code here.
	
	if(curenv != NULL){
		int curid = ENVX(curenv->env_id);
		int i = (curid + 1) % NENV;

		uint32_t max_prio = 0;
		int torun = -1;
		while(i != curid){
			if(envs[i].env_type != ENV_TYPE_IDLE &&
				envs[i].env_status == ENV_RUNNABLE &&
				envs[i].env_prio > max_prio){
				
				max_prio = envs[i].env_prio;
				torun = i;
			}
			i = (i+1) % NENV;
		}

		if(torun > -1 && torun != curid)
			env_run(envs + torun);

		if(curenv->env_status == ENV_RUNNING)
			env_run(curenv);
	}


	/*
	if(curenv == NULL)
		i = 0;
	else
		i = (ENVX(curenv->env_id) + 1) % NENV;

	int cnt = 0;
	uint32_t max_prio = 0;
	int torun = -1;
	while(cnt < NENV){
		if(envs[i].env_type != ENV_TYPE_IDLE &&
			envs[i].env_status == ENV_RUNNABLE &&
			envs[i].env_prio > max_prio){
			
			max_prio = envs[i].env_prio;
			torun = i;
			break;
			//env_run(envs + i);
		}

		i = (i+1) % NENV;
		cnt += 1;
	}
	
	cprintf("TORUNNNNNNNNNNNNN: %d\n", torun);

	if(torun > -1){
		if(!curenv || (torun != ENVX(curenv->env_id)))
			env_run(envs + torun);
	}

	if(curenv && curenv->env_status == ENV_RUNNING)
		env_run(curenv);
	*/

	// For debugging and testing purposes, if there are no
	// runnable environments other than the idle environments,
	// drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if (envs[i].env_type != ENV_TYPE_IDLE &&
		    (envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING))
			break;
	}
	if (i == NENV) {
		cprintf("No more runnable environments!\n");
		while (1)
			monitor(NULL);
	}

	// Run this CPU's idle environment when nothing else is runnable.
	idle = &envs[cpunum()];
	if (!(idle->env_status == ENV_RUNNABLE || idle->env_status == ENV_RUNNING))
		panic("CPU %d: No idle environment!", cpunum());
	env_run(idle);
}
