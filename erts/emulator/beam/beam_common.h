/*
 * %CopyrightBegin%
 *
 * Copyright Ericsson AB 1996-2020. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * %CopyrightEnd%
 */

#ifndef _BEAM_COMMON_H_
#define _BEAM_COMMON_H_

#if defined(HARDDEBUG)

#  define CHECK_TERM(T) size_object(T)

#  define CHECK_ARGS(PC)                 \
do {                                     \
  int i_;                                \
  int Arity_ = PC[-1];                   \
  for (i_ = 0; i_ < Arity_; i_++) {      \
	CHECK_TERM(x(i_));               \
  }                                      \
} while (0)

#else
#  define CHECK_TERM(T) ASSERT(!is_CP(T))
#  define CHECK_ARGS(T)
#endif

#ifdef DEBUG

#ifdef BEAMASM

#define ERTS_DBG_CHK_REDS(P, FC)					\
    do {								\
	    ASSERT(FC <= CONTEXT_REDS);					\
	    ASSERT(erts_proc_sched_data(c_p)->virtual_reds		\
		   <= CONTEXT_REDS - (FC));				\
} while (0)
#else
#define ERTS_DBG_CHK_REDS(P, FC)					\
    do {								\
	if (ERTS_PROC_GET_SAVED_CALLS_BUF((P))) {			\
	    ASSERT(FC <= 0);						\
	    ASSERT(erts_proc_sched_data(c_p)->virtual_reds		\
		   <= 0 - (FC));					\
	}								\
	else {								\
	    ASSERT(FC <= CONTEXT_REDS);					\
	    ASSERT(erts_proc_sched_data(c_p)->virtual_reds		\
		   <= CONTEXT_REDS - (FC));				\
	}								\
} while (0)
#endif

#else
#define ERTS_DBG_CHK_REDS(P, FC)
#endif

#define SWAPIN             \
    HTOP = HEAP_TOP(c_p);  \
    E = c_p->stop

#define SWAPOUT            \
    HEAP_TOP(c_p) = HTOP;  \
    c_p->stop = E

#define HEAVY_SWAPIN       \
    SWAPIN;		   \
    FCALLS = c_p->fcalls

#define HEAVY_SWAPOUT      \
    SWAPOUT;		   \
    c_p->fcalls = FCALLS

#ifdef ERTS_ENABLE_LOCK_CHECK
#    define PROCESS_MAIN_CHK_LOCKS(P)                   \
do {                                                    \
    if ((P))                                            \
	erts_proc_lc_chk_only_proc_main((P));           \
    ERTS_LC_ASSERT(!erts_thr_progress_is_blocking());   \
} while (0)
#    define ERTS_REQ_PROC_MAIN_LOCK(P)				\
do {                                                            \
    if ((P))                                                    \
	erts_proc_lc_require_lock((P), ERTS_PROC_LOCK_MAIN,     \
				  __FILE__, __LINE__);          \
} while (0)
#    define ERTS_UNREQ_PROC_MAIN_LOCK(P)				\
do {									\
    if ((P))								\
	erts_proc_lc_unrequire_lock((P), ERTS_PROC_LOCK_MAIN);		\
} while (0)
#else
#  define PROCESS_MAIN_CHK_LOCKS(P)
#  define ERTS_REQ_PROC_MAIN_LOCK(P)
#  define ERTS_UNREQ_PROC_MAIN_LOCK(P)
#endif

#define x(N) reg[N]
#define y(N) E[N]
#define r(N) x(N)
#define Q(N) (N*sizeof(Eterm *))
#define l(N) (freg[N].fd)

#define Arg(N)       I[(N)+1]

#define GetSource(raw, dst)			\
   do {						\
     dst = raw;                                 \
     switch (loader_tag(dst)) {			\
     case LOADER_X_REG:				\
        dst = x(loader_x_reg_index(dst));       \
        break;					\
     case LOADER_Y_REG:				\
        ASSERT(loader_y_reg_index(dst) >= 1);	\
        dst = y(loader_y_reg_index(dst));       \
        break;					\
     }						\
     CHECK_TERM(dst);				\
   } while (0)

#define PUT_TERM_REG(term, desc)		\
do {						\
    switch (loader_tag(desc)) {			\
    case LOADER_X_REG:				\
	x(loader_x_reg_index(desc)) = (term);	\
	break;					\
    case LOADER_Y_REG:				\
	y(loader_y_reg_index(desc)) = (term);	\
	break;					\
    default:					\
	ASSERT(0);				\
	break;					\
    }						\
} while(0)

#ifdef USE_VM_PROBES
#  define USE_VM_CALL_PROBES
#endif

#ifdef USE_VM_CALL_PROBES

#define DTRACE_LOCAL_CALL(p, mfa)					\
    if (DTRACE_ENABLED(local_function_entry)) {				\
        DTRACE_CHARBUF(process_name, DTRACE_TERM_BUF_SIZE);		\
        DTRACE_CHARBUF(mfa_buf, DTRACE_TERM_BUF_SIZE);			\
        int depth = STACK_START(p) - STACK_TOP(p);			\
        dtrace_fun_decode(p, mfa, process_name, mfa_buf);               \
        DTRACE3(local_function_entry, process_name, mfa_buf, depth);	\
    }

#define DTRACE_GLOBAL_CALL(p, mfa)					\
    if (DTRACE_ENABLED(global_function_entry)) {			\
        DTRACE_CHARBUF(process_name, DTRACE_TERM_BUF_SIZE);		\
        DTRACE_CHARBUF(mfa_buf, DTRACE_TERM_BUF_SIZE);			\
        int depth = STACK_START(p) - STACK_TOP(p);			\
        dtrace_fun_decode(p, mfa, process_name, mfa_buf);               \
        DTRACE3(global_function_entry, process_name, mfa_buf, depth);	\
    }

#define DTRACE_RETURN(p, mfa)                                    \
    if (DTRACE_ENABLED(function_return)) {                      \
        DTRACE_CHARBUF(process_name, DTRACE_TERM_BUF_SIZE);     \
        DTRACE_CHARBUF(mfa_buf, DTRACE_TERM_BUF_SIZE);          \
        int depth = STACK_START(p) - STACK_TOP(p);              \
        dtrace_fun_decode(p, mfa, process_name, mfa_buf);       \
        DTRACE3(function_return, process_name, mfa_buf, depth); \
    }

#define DTRACE_BIF_ENTRY(p, mfa)                                    \
    if (DTRACE_ENABLED(bif_entry)) {                                \
        DTRACE_CHARBUF(process_name, DTRACE_TERM_BUF_SIZE);         \
        DTRACE_CHARBUF(mfa_buf, DTRACE_TERM_BUF_SIZE);              \
        dtrace_fun_decode(p, mfa, process_name, mfa_buf);           \
        DTRACE2(bif_entry, process_name, mfa_buf);                  \
    }

#define DTRACE_BIF_RETURN(p, mfa)                                   \
    if (DTRACE_ENABLED(bif_return)) {                               \
        DTRACE_CHARBUF(process_name, DTRACE_TERM_BUF_SIZE);         \
        DTRACE_CHARBUF(mfa_buf, DTRACE_TERM_BUF_SIZE);              \
        dtrace_fun_decode(p, mfa, process_name, mfa_buf);           \
        DTRACE2(bif_return, process_name, mfa_buf);                 \
    }

#define DTRACE_NIF_ENTRY(p, mfa)                                        \
    if (DTRACE_ENABLED(nif_entry)) {                                    \
        DTRACE_CHARBUF(process_name, DTRACE_TERM_BUF_SIZE);             \
        DTRACE_CHARBUF(mfa_buf, DTRACE_TERM_BUF_SIZE);                  \
        dtrace_fun_decode(p, mfa, process_name, mfa_buf);               \
        DTRACE2(nif_entry, process_name, mfa_buf);                      \
    }

#define DTRACE_NIF_RETURN(p, mfa)                                       \
    if (DTRACE_ENABLED(nif_return)) {                                   \
        DTRACE_CHARBUF(process_name, DTRACE_TERM_BUF_SIZE);             \
        DTRACE_CHARBUF(mfa_buf, DTRACE_TERM_BUF_SIZE);                  \
        dtrace_fun_decode(p, mfa, process_name, mfa_buf);               \
        DTRACE2(nif_return, process_name, mfa_buf);                     \
    }

#define DTRACE_GLOBAL_CALL_FROM_EXPORT(p,e)                                                    \
    do {                                                                                       \
        if (DTRACE_ENABLED(global_function_entry)) {                                           \
            BeamInstr* fp = (BeamInstr *) (((Export *) (e))->addressv[erts_active_code_ix()]); \
            DTRACE_GLOBAL_CALL((p), erts_code_to_codemfa(fp));          \
        }                                                                                      \
    } while(0)

#define DTRACE_RETURN_FROM_PC(p, i)                                                        \
    do {                                                                                \
        ErtsCodeMFA* cmfa;                                                                  \
        if (DTRACE_ENABLED(function_return) && (cmfa = erts_find_function_from_pc(i))) { \
            DTRACE_RETURN((p), cmfa);                               \
        }                                                                               \
    } while(0)

#else /* USE_VM_PROBES */
#define DTRACE_LOCAL_CALL(p, mfa)        do {} while (0)
#define DTRACE_GLOBAL_CALL(p, mfa)       do {} while (0)
#define DTRACE_GLOBAL_CALL_FROM_EXPORT(p, e) do {} while (0)
#define DTRACE_RETURN(p, mfa)            do {} while (0)
#define DTRACE_RETURN_FROM_PC(p, i)      do {} while (0)
#define DTRACE_BIF_ENTRY(p, mfa)         do {} while (0)
#define DTRACE_BIF_RETURN(p, mfa)        do {} while (0)
#define DTRACE_NIF_ENTRY(p, mfa)         do {} while (0)
#define DTRACE_NIF_RETURN(p, mfa)        do {} while (0)
#endif /* USE_VM_PROBES */

/*
 * We reuse some of fields in the save area in the process structure.
 * This is safe to do, since this space is only actively used when
 * the process is switched out.
 */
#define REDS_IN(p)  ((p)->def_arg_reg[5])

ErtsCodeMFA *ubif2mfa(void* uf);
BeamInstr* handle_error(Process* c_p, BeamInstr* pc,
                        Eterm* reg, ErtsCodeMFA* bif_mfa);
BeamInstr* call_error_handler(Process* p, BeamInstr* I,
                              Eterm* reg, Eterm func);
BeamInstr* fixed_apply(Process* p, Eterm* reg, Uint arity,
			BeamInstr *I, Uint offs);
BeamInstr* apply(Process* p, Eterm* reg,
                        BeamInstr *I, Uint offs);
BeamInstr* call_fun(Process* p, int arity,
			   Eterm* reg, Eterm args);
BeamInstr* apply_fun(Process* p, Eterm fun,
			    Eterm args, Eterm* reg);
Eterm new_fun(Process* p, Eterm* reg,
		     ErlFunEntry* fe, int num_free);
ErlFunThing* new_fun_thing(Process* p, ErlFunEntry* fe, int num_free);
int is_function2(Eterm Term, Uint arity);
Eterm erts_gc_new_map(Process* p, Eterm* reg, Uint live,
                             Uint n, BeamInstr* ptr);
Eterm erts_gc_new_small_map_lit(Process* p, Eterm* reg, Eterm keys_literal,
                               Uint live, BeamInstr* ptr);
Eterm erts_gc_update_map_assoc(Process* p, Eterm* reg, Uint live,
                              Uint n, BeamInstr* new_p);
Eterm erts_gc_update_map_exact(Process* p, Eterm* reg, Uint live,
                              Uint n, Eterm* new_p);
Eterm get_map_element(Eterm map, Eterm key);
Eterm get_map_element_hash(Eterm map, Eterm key, Uint32 hx);
int raw_raise(Eterm stacktrace, Eterm exc_class, Eterm value, Process *c_p);
void erts_sanitize_freason(Process* c_p, Eterm exc);
Eterm add_stacktrace(Process* c_p, Eterm Value, Eterm exc);
void copy_out_registers(Process *c_p, Eterm *reg);
void copy_in_registers(Process *c_p, Eterm *reg);
void check_monitor_long_schedule(Process *c_p, Uint64 start_time, BeamInstr* start_time_i);


#define BeamCodeApply() beam_apply

#ifdef BEAMASM
#define BeamCodeNormalExit() beam_normal_exit
extern BeamInstr *beam_apply;
extern BeamInstr *beam_normal_exit;
extern BeamInstr *beam_exit;
extern BeamInstr *beam_save_calls;
extern BeamInstr *beam_continue_exit;
extern BeamInstr *beam_return_to_trace;   /* OpCode(i_return_to_trace) */
extern BeamInstr *beam_return_trace;      /* OpCode(i_return_trace) */
extern BeamInstr *beam_exception_trace;   /* OpCode(i_exception_trace) */
extern BeamInstr *beam_return_time_trace; /* OpCode(i_return_time_trace) */
#else
#define BeamCodeNormalExit() (beam_apply + 1)
extern BeamInstr beam_apply[2];
extern BeamInstr beam_exit[1];
extern BeamInstr beam_continue_exit[1];
extern BeamInstr beam_return_to_trace[1];   /* OpCode(i_return_to_trace) */
extern BeamInstr beam_return_trace[1];      /* OpCode(i_return_trace) */
extern BeamInstr beam_exception_trace[1];   /* OpCode(i_exception_trace) */
extern BeamInstr beam_return_time_trace[1]; /* OpCode(i_return_time_trace) */
#endif

#endif /* _BEAM_COMMON_H_ */
