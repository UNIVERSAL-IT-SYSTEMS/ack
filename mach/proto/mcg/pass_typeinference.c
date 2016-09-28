#include "mcg.h"

static ARRAYOF(struct ir) pending;

static bool interesting(enum ir_type type)
{
    return (type != IRT_ANY) && (type != IRT_UNSET);
}

static void perform_inference(void)
{
    bool changed;
    int i;

    do
    {
        changed = false;

        i = 0;
        while (i < pending.count)
        {
            struct ir* ir = pending.item[i];
            const struct ir_data* data = &ir_data[ir->opcode];

            /* If a branch has a declared type, and the node along that branch
             * doesn't, then that node has the declared type. */

            if (ir->left && interesting(data->lefttype) && !interesting(ir->left->type))
                ir->left->type = data->lefttype;

            if (ir->right && interesting(data->righttype) && !interesting(ir->right->type))
                ir->right->type = data->righttype;

            /* If the opcode has a declared type, that is this ir's type. */

            if (interesting(data->returntype))
                ir->type = data->returntype;

            /* If we have a declared type, and either of our branches doesn't, that branch
             * becomes our type. */

            if (interesting(ir->type))
            {
                if (ir->left && (data->lefttype == IRT_ANY) && (ir->left->type == IRT_UNSET))
                    ir->left->type = ir->type;

                if (ir->right && (data->righttype == IRT_ANY) && (ir->right->type == IRT_UNSET))
                    ir->right->type = ir->type;
            }

            if (ir->type != IRT_UNSET)
            {
                changed = true;
                array_remove(&pending, ir);
            }
            else
                i++;
        }
    }
    while (changed);
}

static void patch_types(void)
{
    int i;

    for (i=0; i<pending.count; i++)
    {
        struct ir* ir = pending.item[i];
        if ((ir->type == IRT_UNSET) && (ir->size != 0))
            ir->type = IRT_INT;

        /* FIXME: should eventually become an option */
        if (ir->type == IRT_PTR)
            ir->type = IRT_INT;
    }
}

static void addall(struct ir* ir)
{
    if (array_appendu(&pending, ir))
        return;

    if (ir->left)
        addall(ir->left);
    if (ir->right)
        addall(ir->right);
}

static void collect_irs(struct procedure* proc)
{
    int i;
    
    pending.count = 0;
	for (i=0; i<proc->blocks.count; i++)
    {
        struct basicblock* bb = proc->blocks.item[i];
        int j;

        for (j=0; j<bb->irs.count; j++)
            addall(bb->irs.item[j]);
    }
}

void pass_type_inference(struct procedure* proc)
{
    collect_irs(proc);
    perform_inference();

    collect_irs(proc);
    patch_types();
}

/* vim: set sw=4 ts=4 expandtab : */


