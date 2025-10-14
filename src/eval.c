
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <gmp.h>

#include "eval.h"
#include "utils.h"
#include "env.h"

void eval_init(eval_context_t *ctx)
{
    ctx->current_scope = env_create(NULL);
}

void eval_free(eval_context_t *ctx)
{
    if (!ctx) return;
    env_free(ctx->current_scope);
}

value_t math_sin(eval_context_t *ctx, size_t argc, value_t *argv)
{
    if (argc != 1 || argv[0].type != VALUE_NUMBER)
        TODO("Math.sin expects 1 numeric argument");

    number_t result;
    mpfr_init2(result, MPFR_PRECISION);

    mpfr_sin(result, argv[0].number, MPFR_RNDN);

    value_t val = value_number(result);
    mpfr_clear(result);
    return val;
}

value_t math_cos(eval_context_t *ctx, size_t argc, value_t *argv)
{
    if (argc != 1 || argv[0].type != VALUE_NUMBER)
        TODO("Math.cos expects 1 numeric argument");

    number_t result;
    mpfr_init2(result, MPFR_PRECISION);

    mpfr_cos(result, argv[0].number, MPFR_RNDN);

    value_t val = value_number(result);
    mpfr_clear(result);
    return val;
}

value_t math_tan(eval_context_t *ctx, size_t argc, value_t *argv)
{
    if (argc != 1 || argv[0].type != VALUE_NUMBER)
        TODO("Math.tan expects 1 numeric argument");

    number_t result;
    mpfr_init2(result, MPFR_PRECISION);

    mpfr_tan(result, argv[0].number, MPFR_RNDN);

    value_t val = value_number(result);
    mpfr_clear(result);
    return val;
}

value_t math_asin(eval_context_t *ctx, size_t argc, value_t *argv)
{
    if (argc != 1 || argv[0].type != VALUE_NUMBER)
        TODO("Math.asin expects 1 numeric argument");

    number_t result;
    mpfr_init2(result, MPFR_PRECISION);

    mpfr_asin(result, argv[0].number, MPFR_RNDN);

    value_t val = value_number(result);
    mpfr_clear(result);
    return val;
}

value_t math_acos(eval_context_t *ctx, size_t argc, value_t *argv)
{
    if (argc != 1 || argv[0].type != VALUE_NUMBER)
        TODO("Math.acos expects 1 numeric argument");

    number_t result;
    mpfr_init2(result, MPFR_PRECISION);
    mpfr_acos(result, argv[0].number, MPFR_RNDN);

    value_t val = value_number(result);
    mpfr_clear(result);
    return val;
}

value_t math_atan(eval_context_t *ctx, size_t argc, value_t *argv)
{
    if (argc != 1 || argv[0].type != VALUE_NUMBER)
        TODO("Math.atan expects 1 numeric argument");

    number_t result;
    mpfr_init2(result, MPFR_PRECISION);
    mpfr_atan(result, argv[0].number, MPFR_RNDN);

    value_t val = value_number(result);
    mpfr_clear(result);
    return val;
}

value_t math_sqrt(eval_context_t *ctx, size_t argc, value_t *argv)
{
    if (argc != 1 || argv[0].type != VALUE_NUMBER)
        TODO("Math.sqrt expects 1 numeric argument");

    if (mpfr_sgn(argv[0].number) < 0)
        TODO("Math.sqrt cannot take negative numbers");

    number_t result;
    mpfr_init2(result, MPFR_PRECISION);
    mpfr_sqrt(result, argv[0].number, MPFR_RNDN);

    value_t val = value_number(result);
    mpfr_clear(result);
    return val;
}

value_t math_log(eval_context_t *ctx, size_t argc, value_t *argv)
{
    if (argc != 1 || argv[0].type != VALUE_NUMBER)
        TODO("Math.log expects 1 numeric argument");

    if (mpfr_sgn(argv[0].number) <= 0)
        TODO("Math.log cannot take non-positive numbers");

    number_t result;
    mpfr_init2(result, MPFR_PRECISION);
    mpfr_log(result, argv[0].number, MPFR_RNDN);

    value_t val = value_number(result);
    mpfr_clear(result);
    return val;
}

value_t math_exp(eval_context_t *ctx, size_t argc, value_t *argv)
{
    if (argc != 1 || argv[0].type != VALUE_NUMBER)
        TODO("Math.exp expects 1 numeric argument");

    number_t result;
    mpfr_init2(result, MPFR_PRECISION);
    mpfr_exp(result, argv[0].number, MPFR_RNDN);

    value_t val = value_number(result);
    mpfr_clear(result);
    return val;
}

value_t math_abs(eval_context_t *ctx, size_t argc, value_t *argv)
{
    if (argc != 1 || argv[0].type != VALUE_NUMBER)
        TODO("Math.abs expects 1 numeric argument");

    number_t result;
    mpfr_init2(result, MPFR_PRECISION);
    mpfr_abs(result, argv[0].number, MPFR_RNDN);

    value_t val = value_number(result);
    mpfr_clear(result);
    return val;
}

value_t math_min(eval_context_t *ctx, size_t argc, value_t *argv)
{
    if (argc == 0)
        TODO("Math.min expects at least 1 argument");

    for (size_t i = 0; i < argc; i++) {
        if (argv[i].type != VALUE_NUMBER)
            TODO("Math.min expects numeric arguments only");
    }

    number_t result;
    mpfr_init2(result, MPFR_PRECISION);
    mpfr_set(result, argv[0].number, MPFR_RNDN);

    for (size_t i = 1; i < argc; i++) {
        if (mpfr_less_p(argv[i].number, result)) {
            mpfr_set(result, argv[i].number, MPFR_RNDN);
        }
    }

    value_t val = value_number(result);
    mpfr_clear(result);
    return val;
}

value_t math_max(eval_context_t *ctx, size_t argc, value_t *argv)
{
    if (argc == 0)
        TODO("Math.max expects at least 1 argument");

    for (size_t i = 0; i < argc; i++) {
        if (argv[i].type != VALUE_NUMBER)
            TODO("Math.max expects numeric arguments only");
    }

    number_t result;
    mpfr_init2(result, MPFR_PRECISION);
    mpfr_set(result, argv[0].number, MPFR_RNDN);

    for (size_t i = 1; i < argc; i++) {
        if (mpfr_greater_p(argv[i].number, result)) {
            mpfr_set(result, argv[i].number, MPFR_RNDN);
        }
    }

    value_t val = value_number(result);
    mpfr_clear(result);
    return val;
}
value_t math_sign(eval_context_t *ctx, size_t argc, value_t *argv)
{
    if (argc != 1 || argv[0].type != VALUE_NUMBER)
        TODO("Math.sign expects 1 numeric argument");

    int sgn = mpfr_sgn(argv[0].number);

    number_t result;
    mpfr_init2(result, MPFR_PRECISION);
    if (sgn > 0) mpfr_set_ui(result, 1, MPFR_RNDN);
    else if (sgn < 0) mpfr_set_si(result, -1, MPFR_RNDN);
    else mpfr_set_ui(result, 0, MPFR_RNDN);

    value_t val = value_number(result);
    mpfr_clear(result);
    return val;
}

value_t math_random(eval_context_t *ctx, size_t argc, value_t *argv)
{
    (void)ctx; (void)argc; (void)argv;  // ignore arguments

    number_t result;
    mpfr_init2(result, MPFR_PRECISION);

    // Seed only once
    static bool seeded = false;
    if (!seeded) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        unsigned int seed = (unsigned int)(ts.tv_nsec ^ ts.tv_sec);
        srand(seed);
        seeded = true;
    }

    double r = (double)rand() / (double)RAND_MAX;
    mpfr_set_d(result, r, MPFR_RNDN);

    value_t val = value_number(result);
    mpfr_clear(result);
    return val;
}

value_t math_floor(eval_context_t *ctx, size_t argc, value_t *argv)
{
    if (argc != 1 || argv[0].type != VALUE_NUMBER)
        TODO("Math.floor expects 1 numeric argument");

    number_t result;
    mpfr_init2(result, MPFR_PRECISION);

    mpfr_floor(result, argv[0].number);  // compute floor

    value_t val = value_number(result);
    mpfr_clear(result);
    return val;
}

value_t math_ceil(eval_context_t *ctx, size_t argc, value_t *argv)
{
    if (argc != 1 || argv[0].type != VALUE_NUMBER)
        TODO("Math.ceil expects 1 numeric argument");

    number_t result;
    mpfr_init2(result, MPFR_PRECISION);

    mpfr_ceil(result, argv[0].number);  // compute ceil

    value_t val = value_number(result);
    mpfr_clear(result);
    return val;
}


void math_add_function(object_t *obj, const char *name, value_t (*func)(eval_context_t *ctx, size_t argc, value_t *argv))
{
    function_t *fn = malloc(sizeof(function_t));
    fn->is_native = true;
    fn->native_ptr = func;
    object_set(obj, name, value_function(fn));
}

value_t eval_program(eval_context_t *ctx, node_t *program)
{
    /* add build-ins */
    value_t math_obj = value_object_create();

    // PI
    number_t pi;
    mpfr_init2(pi, MPFR_PRECISION);     // set desired precision
    mpfr_const_pi(pi, MPFR_RNDN);       // set pi
    object_set(&math_obj.object, "PI", value_number(pi));
    mpfr_clear(pi); // clear after wrapping into value_t
    
    // E
    number_t e;
    mpfr_init2(e, MPFR_PRECISION);
    mpfr_set_ui(e, 1, MPFR_RNDN);       // e = 1
    mpfr_exp(e, e, MPFR_RNDN);          // e = exp(1)
    object_set(&math_obj.object, "E", value_number(e));
    mpfr_clear(e); // clear after wrapping

    // PHI = (1 + sqrt(5)) / 2
    number_t phi, tmp;
    mpfr_init2(phi, MPFR_PRECISION);
    mpfr_init2(tmp, MPFR_PRECISION);

    // tmp = sqrt(5)
    mpfr_set_ui(tmp, 5, MPFR_RNDN);
    mpfr_sqrt(tmp, tmp, MPFR_RNDN);

    // phi = 1 + sqrt(5)
    mpfr_set_ui(phi, 1, MPFR_RNDN);
    mpfr_add(phi, phi, tmp, MPFR_RNDN);

    // phi = phi / 2
    mpfr_div_ui(phi, phi, 2, MPFR_RNDN);

    object_set(&math_obj.object, "PHI", value_number(phi));

    mpfr_clear(phi);
    mpfr_clear(tmp);

    // Trigonometric
    math_add_function(&math_obj.object, "sin", math_sin);

    math_add_function(&math_obj.object, "cos", math_cos);
    math_add_function(&math_obj.object, "tan", math_tan);
    math_add_function(&math_obj.object, "asin", math_asin);
    math_add_function(&math_obj.object, "acos", math_acos);
    math_add_function(&math_obj.object, "atan", math_atan);
    // math_add_function(&math_obj.object, "atan2", math_atan2);

    // Hyperbolic
    // math_add_function(&math_obj.object, "sinh", math_sinh);
    // math_add_function(&math_obj.object, "cosh", math_cosh);
    // math_add_function(&math_obj.object, "tanh", math_tanh);

    // Exponential / logarithmic
    math_add_function(&math_obj.object, "exp", math_exp);
    math_add_function(&math_obj.object, "log", math_log);
    // math_add_function(&math_obj.object, "log10", math_log10);
    // math_add_function(&math_obj.object, "log2", math_log2);

    // Power / roots
    math_add_function(&math_obj.object, "sqrt", math_sqrt);
    // math_add_function(&math_obj.object, "cbrt", math_cbrt);
    // math_add_function(&math_obj.object, "pow", math_pow);

    // Rounding / absolute
    math_add_function(&math_obj.object, "abs", math_abs);
    math_add_function(&math_obj.object, "floor", math_floor);
    math_add_function(&math_obj.object, "ceil", math_ceil);
    // math_add_function(&math_obj.object, "round", math_round);
    // math_add_function(&math_obj.object, "trunc", math_trunc);

    // Min / Max
    math_add_function(&math_obj.object, "min", math_min);
    math_add_function(&math_obj.object, "max", math_max);

    // Sign / random
    math_add_function(&math_obj.object, "sign", math_sign);
    math_add_function(&math_obj.object, "random", math_random);
    
    /* set the `Math` object */
    env_set(ctx->current_scope, "Math", math_obj);

    value_t result = value_undefined();
    if (!program) return result;

    /* hoisting */
    for (size_t i = 0; i < program->program.count; i++)
    {
        node_t *stmt = program->program.statements[i];
        /* hoisting only functions and vars */
        switch (stmt->type)
        {
            case NODE_FUNCTION:
                /* define function */
                break;
            case NODE_DECLARATION:
                if (stmt->declaration.kind.type == TOKEN_VAR)
                {
                    for (size_t j = 0; j < stmt->declaration.count; j++)
                    {
                        /* define var */
                    }
                }
                break;
            default:
                break;
        }
    }

    for (size_t i = 0; i < program->program.count; i++)
    {
        result = eval_node(ctx, program->program.statements[i]);
    }

    return result;
}

value_t eval_node(eval_context_t *ctx, node_t *node)
{
    if (!node) return value_undefined();

    switch (node->type)
    {
        case NODE_NUMBER:
            return value_number(node->number);

        case NODE_STRING:
            return value_string(node->string);

        case NODE_BOOL:
            return value_bool(node->boolean);

        case NODE_IDENTIFIER: {
            variable_t *ident = env_get(ctx->current_scope, node->identifier);
            if (!ident) TODO("Identifier '%s' not found", node->identifier);
            return ident->value;
        }

        case NODE_ARRAY:
            TODO("NODE_ARRAY not implemented");

        case NODE_OBJECT:
            TODO("NODE_OBJECT not implemented");

        case NODE_SPREAD:
            TODO("NODE_SPREAD not implemented");

        case NODE_UNDEFINED:
            return value_undefined();

        case NODE_NULL:
            return value_null();

        case NODE_PROGRAM:
            TODO("NODE_PROGRAM should be handled by eval_program");

        case NODE_BLOCK: {
            env_enter_scope(ctx->current_scope);

            value_t result = value_undefined();

            /* hoisting */
            for (size_t i = 0; i < node->block.count; i++)
            {
                node_t *stmt = node->block.statements[i];
                /* hoisting only functions and vars */
                switch (stmt->type)
                {
                    case NODE_FUNCTION:
                        /* define function */
                        break;
                    case NODE_DECLARATION:
                        if (stmt->declaration.kind.type == TOKEN_VAR)
                        {
                            for (size_t j = 0; j < stmt->declaration.count; j++)
                            {
                                /* define var */
                            }
                        }
                        break;
                    default:
                        break;
                }
            }

            /* execute statements */
            for (size_t i = 0; i < node->block.count; i++)
            {
                result = eval_node(ctx, node->block.statements[i]);
            }

            env_leave_scope(ctx->current_scope);

            return result;
        }

        case NODE_BINARY: {
            value_t left = eval_node(ctx, node->binary.left);
            value_t right = eval_node(ctx, node->binary.right);

            switch (node->binary.op.type)
            {
                case TOKEN_PLUS: {
                    if (left.type == VALUE_NUMBER && right.type == VALUE_NUMBER)
                    {
                        number_t result;
                        mpfr_init2(result, MPFR_PRECISION);
                        mpfr_add(result, left.number, right.number, MPFR_RNDN);

                        value_t val = value_number(result);
                        mpfr_clear(result);
                        return val;
                    }

                    TODO("Unsupported '+' for %d, %d", left.type, right.type);
                    break;
                }
                case TOKEN_MINUS: {
                    if (left.type == VALUE_NUMBER && right.type == VALUE_NUMBER)
                    {
                        number_t result;
                        mpfr_init2(result, MPFR_PRECISION);
                        mpfr_sub(result, left.number, right.number, MPFR_RNDN);

                        value_t val = value_number(result);
                        mpfr_clear(result);
                        return val;
                    }

                    TODO("Unsupported '-' for %d, %d", left.type, right.type);
                    break;
                }
                case TOKEN_STAR: {
                    if (left.type == VALUE_NUMBER && right.type == VALUE_NUMBER)
                    {
                        number_t result;
                        mpfr_init2(result, MPFR_PRECISION);
                        mpfr_mul(result, left.number, right.number, MPFR_RNDN);

                        value_t val = value_number(result);
                        mpfr_clear(result);
                        return val;
                    }

                    TODO("Unsupported '*' for %d, %d", left.type, right.type);
                    break;
                }
                case TOKEN_SLASH: {
                    if (left.type == VALUE_NUMBER && right.type == VALUE_NUMBER) {
                        // handle divide-by-zero check
                        if (mpfr_zero_p(right.number)) {
                            TODO("Division by zero");
                        }
                        number_t result;
                        mpfr_init2(result, MPFR_PRECISION);
                        mpfr_div(result, left.number, right.number, MPFR_RNDN);

                        value_t val = value_number(result);
                        mpfr_clear(result);
                        return val;
                    }
                    TODO("Unsupported '/' for %d, %d", left.type, right.type);
                    break;
                }
                case TOKEN_PERCENT: {
                    if (left.type == VALUE_NUMBER && right.type == VALUE_NUMBER) {
                        if (mpfr_zero_p(right.number)) {
                            TODO("Modulo by zero");
                        }
                        number_t result;
                        mpfr_init2(result, MPFR_PRECISION);
                        mpfr_fmod(result, left.number, right.number, MPFR_RNDN);

                        value_t val = value_number(result);
                        mpfr_clear(result);
                        return val;
                    }
                    TODO("Unsupported '%%' for %d, %d", left.type, right.type);
                    break;
                }
                case TOKEN_STAR_STAR: {
                    if (left.type == VALUE_NUMBER && right.type == VALUE_NUMBER)
                    {
                        number_t result;
                        mpfr_init2(result, MPFR_PRECISION);
                        mpfr_pow(result, left.number, right.number, MPFR_RNDN);

                        value_t val = value_number(result);
                        mpfr_clear(result);
                        return val;
                    }

                    TODO("Unsupported '**' for %d, %d", left.type, right.type);
                    break;
                }
                default: {
                    TODO("Unimplemented binary operator %d", node->binary.op.type);
                }
            }
            UNREACHABLE;
        }

        case NODE_UNARY:
            TODO("NODE_UNARY not implemented");

        case NODE_ASSIGNMENT:
            TODO("NODE_ASSIGNMENT not implemented");

        case NODE_TERNARY:
            TODO("NODE_TERNARY not implemented");

        case NODE_IF:
            TODO("NODE_IF not implemented");

        case NODE_WHILE:
            TODO("NODE_WHILE not implemented");

        case NODE_DO_WHILE:
            TODO("NODE_DO_WHILE not implemented");

        case NODE_FOR:
            TODO("NODE_FOR not implemented");

        case NODE_CALL: {
            // Evaluate the callee
            value_t callee = eval_node(ctx, node->call.callee);

            if (callee.type != VALUE_FUNCTION) {
                TODO("Trying to call a non-function");
            }

            // Evaluate arguments
            size_t argc = node->call.arg_count;
            value_t *argv = malloc(sizeof(value_t) * argc);
            for (size_t i = 0; i < argc; i++) {
                argv[i] = eval_node(ctx, node->call.args[i]);
            }

            value_t result;

            if (callee.function->is_native) {
                // Call the native C function
                result = callee.function->native_ptr(ctx, argc, argv);
            } else {
                // User-defined function
                // TODO: implement user-defined function call
                TODO("User-defined function calls not implemented yet");
            }

            free(argv);
            return result;
        }

        case NODE_INDEX:
            TODO("NODE_INDEX not implemented");

        case NODE_MEMBER: {
            value_t obj_val = eval_node(ctx, node->member.object);
            if (obj_val.type != VALUE_OBJECT) {
                TODO("Trying to access member of a non-object");
            }
            
            // The member name (right-hand side) should be a string
            const char *key = node->member.property->identifier;

            // Look up the key in the object
            object_t *obj = &obj_val.object;
            for (size_t i = 0; i < obj->count; i++) {
                if (strcmp(obj->keys[i], key) == 0) {
                    return obj->values[i];  // found
                }
            }

            // Member not found
            TODO("Object has no member '%s'", key);
            break;
        }
        case NODE_POSTFIX:
            TODO("NODE_POSTFIX not implemented");

        case NODE_FUNCTION:
            TODO("NODE_FUNCTION not implemented");

        case NODE_DECLARATION:
            TODO("NODE_DECLARATION not implemented");

        case NODE_SWITCH:
            TODO("NODE_SWITCH not implemented");

        case NODE_LABEL:
            TODO("NODE_LABEL not implemented");

        case NODE_AWAIT:
            TODO("NODE_AWAIT not implemented");

        case NODE_BREAK:
            TODO("NODE_BREAK not implemented");

        case NODE_CONTINUE:
            TODO("NODE_CONTINUE not implemented");

        case NODE_THROW:
            TODO("NODE_THROW not implemented");

        case NODE_TRY:
            TODO("NODE_TRY not implemented");

        case NODE_RETURN:
            TODO("NODE_RETURN not implemented");

        case NODE_IMPORT:
            TODO("NODE_IMPORT not implemented");

        case NODE_EXPORT:
            TODO("NODE_EXPORT not implemented");

        case NODE_EMPTY:
            return value_undefined();

        default:
            TODO("Unimplemented node type %s(%d)", node_type_to_string(node->type), node->type);
    }

    UNREACHABLE;
}
