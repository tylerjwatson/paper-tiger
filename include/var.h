/**
 * Encapsulates the type of the value this cvar contains
 */
typedef enum ptVarType {
    kPtVarTypeUndefined,
    kPtVarTypeNumber,
    kPtVarTypeString,
    kPtVarTypeBoolean,
} ptVarType;


typedef struct ptVar {
    char *name;
    char *value;
} ptVar;

int
ptVarRegister(ptVar *var);

int ptVarDefine()
