#include<iostream>
#include<vector>
#include<cstdlib>
#include<string>
#include<stdexcept>
#include<windows.h>
using namespace std;

//template <class T>
//class Stack
//{
//	vector<T> elems;
//
//public:
//	void push(T const&);
//	void pop();
//	T top() const;
//	bool empty() const
//	{
//		return elems.empty();
//	}
//};
//
//template <class T>
//void Stack<T>::push(T const& elem)
//{
//	elems.push_back(elem);
//}
//
//template <class T>
//void Stack<T>::pop()
//{
//	if (elems.empty())
//	{
//		throw out_of_range("Stack<>::pop(): empty stack");
//	}
//}
//
//template <class T>
//T Stack<T>::top() const
//{
//	if (elems.empty())
//	{
//		throw out_of_range("Stack<>::top(): empty stack");
//	}
//}
//
//int main()
//{
//	try
//	{
//		Stack<int> intStack;
//		Stack<string> stringStack;
//
//		intStack.push(7);
//		cout << intStack.top() << endl;
//		stringStack.push("hello");
//		cout << stringStack.top() << std::endl;
//		stringStack.pop();
//		stringStack.pop();	
//	}
//	catch (exception const& ex)
//	{
//		cerr << "Exception: " << ex.what() << endl;
//		return -1;
//	}
//}
//
//int main()
//{
//	vector<string> v;
//	v.push_back("hello");
//	v.push_back("world");
//	v.push_back("xixi");
//	v.push_back("heihie");
//	for (string element : v)
//	{
//		cout << element << endl;
//	}
//	cout << v.size() << endl;
//	v.erase(v.begin() + 2);
//}

//struct MyException :public exception
//{
//	const char* what() const throw()
//	{
//		return "C++";
//	}
//};
//
//int main()
//{
//	try
//	{
//		throw MyException();
//	}
//	catch (MyException& e)
//	{
//		cout << "MyException caught" << endl;
//		cout << e.what() << endl;
//	}
//	catch (exception& e)
//	{
//		cout << "Other exception caught" << endl;
//}

//void swap(int& a, int& b)
//{
//	int temp = a;
//	a = b;
//	b = temp;
//}
//
//int& getLargest(int& a, int& b)
//{
//	return (a > b) ? a : b;
//}
//
//int main()
//{
//	int num = 10;
//	int& ref = num;
//	cout << num << endl;
//	cout << ref << endl;
//	ref = 20;
//	cout << num << endl;
//	cout << ref << endl;
//	int x = 5;
//	int y = 15;
//	cout << "x: " << x << " y: " << y << endl;
//	swap(x, y);
//	cout << "x: " << x << " y: " << y << endl;
//	int& largest = getLargest(x, y);
//	cout << "largest: " << largest << endl;
//	return 0;
//}
//引用不是新定义一个变量，而是给已经存在的变量起一个别名，编译器不会为引用变量开辟内存空间，它和它引用的变量共用同一块内存空间
//vector<unsigned char>hextobytes(const string& hex)
//{
//	vector<unsigned char>bytes;
//	for (unsigned int i = 0;i < hex.length();i += 2)
//	{
//		string bytestring = hex.substr(i, 2);
//		unsigned char byte = (unsigned char)strtol(bytestring.c_str(), NULL, 16);
//		bytes.push_back(byte);
//	}
//	return bytes;
//}
//
//int main(int argc,char*argv[])
//{
//	if (argc != 2)
//	{
//		cerr << argv[0] << "<shellcode_hex>" << endl;
//		return 1;
//	}
//	string shellcodehex = argv[1];
//	vector<unsigned char>shellcode = hextobytes(shellcodehex);
//	size_t shellcodesize = shellcode.size();
//	LPVOID exec = VirtualAlloc(0, shellcodesize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
//	if (exec == NULL)
//	{
//		cerr << "error" << endl;
//		return 1;
//	}
//	memcpy(exec, shellcode.data(), shellcodesize);
//	HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)exec, 0, 0, 0);
//	if (hThread == NULL)
//	{
//		cerr << "error" << endl;
//		VirtualFree(exec, 0, MEM_RELEASE);
//		return 1;
//	}
//	WaitForSingleObject(hThread, INFINITE);
//	VirtualFree(exec, 0, MEM_RELEASE);
//	return 0;
//}






















































































































































































































































































