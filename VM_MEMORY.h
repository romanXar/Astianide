#pragma once

class VMemory {
private:
	intptr_t* data;
	intptr_t cur = 0;
	const intptr_t CAPACITY = 10000000;

public:
	void SetCur(intptr_t size) { cur = size; }
	void Up(intptr_t size) { cur += size; }
	void Down(intptr_t size) { cur -= size; }

	void SetRelative(intptr_t data_pos, intptr_t value) {
		data[cur + data_pos] = value;
	}

	intptr_t GetRelative(intptr_t data_pos) {
		return data[cur + data_pos];
	}

	void Push(intptr_t value) {
		SetRelative(1, value);
		Up(1);
	}

	intptr_t Pop() {
		intptr_t result = GetRelative(0);
		Down(1);
		return result;
	}

	VMemory() {
		data = (intptr_t*)malloc(CAPACITY * sizeof(intptr_t));
		cur = 0;
	}
	~VMemory() { free(data); }
};
