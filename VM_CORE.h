#pragma once
#include <cstring>

enum
{
	MOV_II_VC,
	MOV_II_R0C,
	MOV_II_R1C,
	MOV_II_R2C,

	ADD_III_VVC,
	ADD_II_VC,

	INC_I,
	INC_R0,
	INC_R0_XMM0,
	INC_R0_XMM1,


	ADD_III_VVV,
	SUB_III_VVV,
	SUB_III_VVC,

	CALL,

	RET,

	IF_LE_II_VC,
	IF_LE_II_R0C,
	IF_LE_II_R0R1,
	IF_LE_II_R0R2,

	EXTERN_CALL,
	EXTERN_CALL_RET,

	GOTO,
	GOTO_R1R2,

	LDA_R1R2,
	LDA_XMM0,
	LDA_XMM1,

	eOF,

	OP_COUNT
};

enum
{
	VAR_I,
	VAR_S,

	R0_I,
	R1_I,
	R2_I,

	LIT_I,
	LIT_S,
};

struct FfiArg
{
	intptr_t type;
	intptr_t value;
};


extern "C" int64_t ffi_call_win64(void* func_ptr, int64_t arg_count, FfiArg* args_array);
__declspec(noinline)
intptr_t* vm_ffi_call(
	intptr_t* cur,
	const std::vector<void*>& extern_funs,
	VMemory& StackFrame,
	std::vector<std::string>& str,
	intptr_t R0, intptr_t R1, intptr_t R2,
	int64_t& result_out)
{
	void* extern_fun = extern_funs[cur[1]];
	intptr_t  count = cur[2];

	std::vector<FfiArg> args;
	args.reserve(count);

	cur += 3;
	for (intptr_t k = 0; k < count; k++)
	{
		if (cur[0] == VAR_I) args.push_back({ 0, StackFrame.GetRelative(cur[1]) });
		else if (cur[0] == LIT_I) args.push_back({ 0, cur[1] });
		else if (cur[0] == LIT_S) args.push_back({ 0, (intptr_t)str[cur[1]].c_str() });
		else if (cur[0] == R0_I)  args.push_back({ 0, R0 });
		else if (cur[0] == R1_I)  args.push_back({ 0, R1 });
		else if (cur[0] == R2_I)  args.push_back({ 0, R2 });
		cur += 2;
	}

	result_out = ffi_call_win64(extern_fun, count, args.data());
	return cur;
}

__declspec(noinline)
intptr_t* vm_extern_call_ret(
	intptr_t* cur,
	const std::vector<void*>& extern_funs,
	VMemory& StackFrame,
	std::vector<std::string>& str,
	intptr_t R0, intptr_t R1, intptr_t R2)
{
	int64_t res;
	cur = vm_ffi_call(cur, extern_funs, StackFrame, str, R0, R1, R2, res);

	if (cur[0] == VAR_I)
		StackFrame.SetRelative(cur[1], res);
	cur += 2;
	return cur;
}
int VM_run(intptr_t bytecode[], size_t bytecode_len, std::vector<std::string>& str)
{

	std::vector<void*> extern_funs;

	HMODULE hMod = LoadLibraryA("msvcrt.dll");
	if (!hMod)
	{
		printf("Failed to get msvcrt.dll\n");
		return 1;
	}

	extern_funs.push_back((void*)GetProcAddress(hMod, "printf"));
	extern_funs.push_back((void*)GetProcAddress(hMod, "clock"));

	VMemory StackFrame;
	VMemory CallFrame;


	void* dispatch_table[OP_COUNT] = {
	&& L_MOV_II_VC,
	&& L_MOV_II_R0C,
	&& L_MOV_II_R1C,
	&& L_MOV_II_R2C,

	&& L_ADD_III_VVC,
	&& L_ADD_II_VC,

	&& L_INC_I,
	&& L_INC_R0,
	&& L_INC_R0_XMM0,
	&& L_INC_R0_XMM1,

	&& L_ADD_III_VVV,
	&& L_SUB_III_VVV,
	&& L_SUB_III_VVC,

	&& L_CALL,

	&& L_RET,

	&& L_IF_LE_II_VC,
	&& L_IF_LE_II_R0C,
	&& L_IF_LE_II_R0R1,
	&& L_IF_LE_II_R0R2,

	&& L_EXTERN_CALL,
	&& L_EXTERN_CALL_RET,
	&& L_GOTO,
	&& L_GOTO_R1R2,

	&& L_LDA_R1R2,
	&& L_LDA_XMM0,
	&& L_LDA_XMM1,

	&& L_eOF
	};


	for (int i = 0; i < bytecode_len; )
	{
		intptr_t op = bytecode[i];
		bytecode[i] = (intptr_t)dispatch_table[op];

		switch (op)
		{
		case MOV_II_VC:       i += 3; break;
		case MOV_II_R0C:       i += 2; break;
		case MOV_II_R1C:       i += 2; break;
		case MOV_II_R2C:       i += 2; break;

		case ADD_III_VVC:
		case ADD_III_VVV:
		case SUB_III_VVC:
		case SUB_III_VVV:     i += 4; break;
		case ADD_II_VC:       i += 3; break;
		case INC_I:           i += 2; break;

		case INC_R0:           i += 1; break;
		case INC_R0_XMM0:      i += 1; break;
		case INC_R0_XMM1:      i += 1; break;



		case GOTO_R1R2:        i += 1; break;
		case LDA_R1R2:
			bytecode[i + 1] = (intptr_t)(bytecode + bytecode[i + 1]);
			i += 2;
			break;
		case LDA_XMM0:
			bytecode[i + 1] = (intptr_t)(bytecode + bytecode[i + 1]);
			i += 2;
			break;
		case LDA_XMM1:
			bytecode[i + 1] = (intptr_t)(bytecode + bytecode[i + 1]);
			i += 2;
			break;
		case CALL:
		case IF_LE_II_VC:
			bytecode[i + 1] = (intptr_t)(bytecode + bytecode[i + 1]);
			i += 4;
			break;
		case IF_LE_II_R0C:
			bytecode[i + 1] = (intptr_t)(bytecode + bytecode[i + 1]);
			i += 3;
			break;
		case IF_LE_II_R0R1:
			bytecode[i + 1] = (intptr_t)(bytecode + bytecode[i + 1]);
			i += 2;
			break;
		case IF_LE_II_R0R2:
			bytecode[i + 1] = (intptr_t)(bytecode + bytecode[i + 1]);
			i += 2;
			break;
		case RET:             i += 2; break;

		case EXTERN_CALL:     i += 3 + 2 * bytecode[i + 2]; break;
		case EXTERN_CALL_RET: i += 3 + 2 * bytecode[i + 2] + 2; break;

		case GOTO:
			bytecode[i + 1] = (intptr_t)(bytecode + bytecode[i + 1]);
			i += 2;
			break;

		case eOF:             i += 1; break;
		default:              i += 1; break;
		}


	}




	  intptr_t jmp_buffer = 0;

	intptr_t* cur = bytecode;
	intptr_t R0, R1, R2;
	R2 = 100000000;
	__m128i xmm0 = _mm_cvtsi64_si128(0);
	__m128i xmm1 = _mm_cvtsi64_si128(0);
	__m128i xmm2 = _mm_cvtsi64_si128(0);


	goto* (void*)cur[0];

L_MOV_II_VC:
	{

		StackFrame.SetRelative(cur[1], cur[2]);
		cur += 3;
		goto* (void*)cur[0];
	}
L_MOV_II_R0C:
	{

		R0 = cur[1];
		cur += 2;
		goto* (void*)cur[0];
	}
L_MOV_II_R1C:
	{

		R1 = cur[1];
		cur += 2;
		goto* (void*)cur[0];
	}
L_MOV_II_R2C:
	{

		R2 = cur[1];
		cur += 2;
		goto* (void*)cur[0];
	}
L_IF_LE_II_VC:
	{

		if (StackFrame.GetRelative(cur[2]) <= cur[3])
		{
			cur += 4;
			goto* (void*)cur[0];
		}
		else
		{
			cur = (intptr_t*)cur[1];
			goto* (void*)cur[0];
		}
	}
L_IF_LE_II_R0C:
	{

		///	if (R0 <= cur[2])
		/*	if (R0 > 100000000)
			{
				cur += 3;
				goto* (void*)cur[0];
			}
			else
			{

				cur = (intptr_t*)R1;
				goto* (void*)R2;


				//cur = (intptr_t*)cur[1];

				//goto* (void*)cur[0];
			}*/

			//if (R0 <= cur[2])
			//if (R0 <= 100000000)
		if (R0 <= R2)
		{
			cur = (intptr_t*)R1;
			//goto* (void*)_mm_cvtsi128_si64(xmm2);

			jmp_buffer = _mm_cvtsi128_si64(xmm2);

			__asm__ __volatile__("" : "+m"(jmp_buffer));
			goto* (void*)jmp_buffer;
		}
		else
		{
			cur += 3;
			goto* (void*)cur[0];



			//cur = (intptr_t*)cur[1];

			//goto* (void*)cur[0];
		}
	}
L_IF_LE_II_R0R1:
	{

		if (R0 <= R1)
		{
			cur += 2;
			goto* (void*)cur[0];
		}
		else
		{
			cur = (intptr_t*)cur[1];
			goto* (void*)cur[0];
		}
	}
L_IF_LE_II_R0R2:
	{

		if (R0 <= R2)
		{
			cur += 2;
			goto* (void*)cur[0];
		}
		else
		{
			cur = (intptr_t*)cur[1];
			goto* (void*)cur[0];
		}
	}
L_ADD_III_VVC:
	{

		StackFrame.SetRelative(cur[1], StackFrame.GetRelative(cur[2]) + cur[3]);
		cur += 4;
		goto* (void*)cur[0];
	}
L_ADD_II_VC:
	{

		StackFrame.SetRelative(cur[1], StackFrame.GetRelative(cur[1]) + cur[2]);
		cur += 3;
		goto* (void*)cur[0];
	}
L_INC_I:
	{

		StackFrame.SetRelative(cur[1], StackFrame.GetRelative(cur[1]) + 1);
		cur += 2;
		goto* (void*)cur[0];
	}
L_INC_R0:
	{

		R0 += 1;
		cur += 1;

		goto* (void*)cur[0];
	}
L_INC_R0_XMM0:
	{
		R0 += 1;
		cur += 1;

		//jmp_buffer = _mm_cvtsi128_si64(xmm0);

		//__asm__ __volatile__("" : "+m"(jmp_buffer));
		//goto* (void*)jmp_buffer;
		goto* (void*)_mm_cvtsi128_si64(xmm0);

	}
L_INC_R0_XMM1:
	{
		R0 += 1;
		cur += 1;

		//jmp_buffer1 = _mm_cvtsi128_si64(xmm1);

		//__asm__ __volatile__("" : "+m"(jmp_buffer1));
		goto* (void*)_mm_cvtsi128_si64(xmm1);
	}
L_ADD_III_VVV:
	{

		StackFrame.SetRelative(cur[1], StackFrame.GetRelative(cur[2]) + StackFrame.GetRelative(cur[3]));
		cur += 4;
		goto* (void*)cur[0];
	}
L_SUB_III_VVV:
	{

		StackFrame.SetRelative(cur[1], StackFrame.GetRelative(cur[2]) - StackFrame.GetRelative(cur[3]));
		cur += 4;
		goto* (void*)cur[0];
	}
L_SUB_III_VVC:
	{

		StackFrame.SetRelative(cur[1], StackFrame.GetRelative(cur[2]) - cur[3]);
		cur += 4;
		goto* (void*)cur[0];
	}
L_EXTERN_CALL:
	{
		int64_t dummy;
		cur = vm_ffi_call(cur, extern_funs, StackFrame, str, R0, R1, R2, dummy);
		goto* (void*)cur[0];
	}

L_EXTERN_CALL_RET:
	{
		cur = vm_extern_call_ret(cur, extern_funs, StackFrame, str, R0, R1, R2);
		goto* (void*)cur[0];
	}
L_GOTO:
	{
		cur = (intptr_t*)cur[1];
		goto* (void*)cur[0];
	}
L_GOTO_R1R2:
	{

		cur = (intptr_t*)R1;
		goto* (void*)R2;


	}
L_LDA_R1R2:
	{
		intptr_t pos = cur[1];
		R1 = (intptr_t)(pos);
		//R2 = *(intptr_t*)pos;
		xmm2 = _mm_cvtsi64_si128(*(intptr_t*)pos);
		cur += 2;
		goto* (void*)cur[0];

	}
L_LDA_XMM0:
	{
		intptr_t pos = cur[1];

		xmm0 = _mm_cvtsi64_si128(*(intptr_t*)pos);

		cur += 2;
		goto* (void*)cur[0];

	}
L_LDA_XMM1:
	{
		intptr_t pos = cur[1];

		xmm1 = _mm_cvtsi64_si128(*(intptr_t*)pos);

		cur += 2;
		goto* (void*)cur[0];

	}
L_CALL:
	{

		intptr_t mem2 = cur[2];

		CallFrame.Push((intptr_t)(cur + 4));
		CallFrame.Push(cur[2]);
		CallFrame.Push(cur[3]);

		cur = (intptr_t*)cur[1];

		StackFrame.Up(mem2);

		goto* (void*)cur[0];
	}
L_RET:
	{

		intptr_t result = StackFrame.GetRelative(cur[1]);

		intptr_t destination = CallFrame.Pop();
		intptr_t popp = CallFrame.Pop();

		intptr_t ret_ptr = CallFrame.Pop();


		StackFrame.Down(popp);

		cur = (intptr_t*)ret_ptr;


		StackFrame.SetRelative(destination, result);

		goto* (void*)cur[0];
	}

L_eOF:
	{

		StackFrame.SetCur(0);
		CallFrame.SetCur(0);
		goto eof;
	}

eof:




	return 0;



}