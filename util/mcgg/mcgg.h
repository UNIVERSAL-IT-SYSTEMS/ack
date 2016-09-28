#ifndef MCGG_H
#define MCGG_H

/* Excruciating macro which packs ir opcodes, sizes and types into an int for
 * iburg's benefit.
 *
 * Sizes are mapped as: 0=0, 1=0, 2=1, 4=2, 8=3.
 * Types are mapped as: INT=0, FLT=1, PTR=2.
 */
#define _encode_size(op, size) \
	((op)*4 + \
		(((size) == 4) ? 2 : \
		 ((size) == 8) ? 3 : \
		 ((size) == 0) ? 0 : \
		  (size-1)))

#define _encode_type(op, type) \
    ((op)*3 + \
        (((type) == IRT_INT) ? 0 : \
         ((type) == IRT_FLT) ? 1 : \
         ((type) == IRT_PTR) ? 2 : \
          0))

#define ir_to_esn(iropcode, size, type) \
    _encode_size(_encode_type(iropcode, type), size)

#define STATE_TYPE void*
typedef struct ir* NODEPTR_TYPE;

#define STATE_LABEL(p) ((p)->state_label)

extern void* burm_label(struct ir* ir);
extern int burm_rule(void* state, int goalnt);
extern const short *burm_nts[];
extern struct ir** burm_kids(struct ir* p, int eruleno, struct ir* kids[]);
extern void burm_trace(struct ir* p, int ruleno, int cost, int bestcost);

struct burm_emitter_data
{
    void (*emit_string)(const char* data);
    void (*emit_reg)(struct ir* ir);
    void (*emit_value)(struct ir* ir);
    void (*emit_resultreg)(void);
    void (*emit_eoi)(void);
    void (*emit_usereg)(struct ir* ir);
};

typedef void burm_emitter_t(struct ir* ir, const struct burm_emitter_data* data);

struct burm_instruction_data
{
    const char* name;
    burm_emitter_t* emitter;
    int allocate;
    bool is_fragment;
};

extern const struct burm_instruction_data burm_instruction_data[];
extern const char* burm_register_class_names[];

#endif

/* vim: set sw=4 ts=4 expandtab : */
