#include <cassert>

template <class Tmp> 
class Stack	{
private:
	int top;
	int capacity;
	Tmp *arr;
	 
public:
	Stack(int cap = 100){
		arr = new Tmp[cap];
		capacity = cap;
		top = -1;
	}
	 
	~Stack(){
		delete[] arr;
	}
	 
	void push(Tmp val){
		if (getload()==capacity){
			std::cout << "Stack.Overflow.\n";
			std::exit(EXIT_FAILURE);
		}
		std::cout << "Stack.Inserting: " << val << std::endl;
		arr[++top] = val;
	}

	Tmp pop(){
		if (getload()==0){
			std::cout << "Stack.Underflow.\n";
			std::exit(EXIT_FAILURE);
		}
		std::cout << "Stack.Removing: " << peek() << std::endl;
		return arr[top--];
	}

	Tmp peek(){
		if (getload()!=0) {
			return arr[top];
		}
		else {
			std::cout << "Stack.Could not peek. \n" ;
		}
	}

	int getload() {		return top + 1;		}
	int getcapacity() {	return capacity; 	}

	Tmp& operator[](int index){
		assert(index >= 0 && index < capacity);
		return arr[index];
	}
};




