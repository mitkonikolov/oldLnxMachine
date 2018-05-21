#include <iostream>
using namespace std;

class Array {

        public:
                double* p;
                int count;
                int size;

                void Initialize();
                void Finalize();
				void Print();

};

void Array::Initialize() {
                p = new double[2];
                count = 0;
                size = 2;
        }
 void Array::Finalize() {
                delete p;
        }

 void Array::Print(){
 				for(int i = 0; i < Array::size; i++){
					cout << p[i] << endl;
					}
 }


int main(){
        Array myArr;
        int choice;
        myArr.Initialize();

		cout << "Main menu:\n\n"
                << "1. Print the array\n"
                << "2. Append element at the end\n"
                << "3. Remove last element\n"
                << "4. Insert one element\n"
                << "5. Exit\n\n"
                << "Select an option: ";
        cin >> choice;

        switch(choice){
                case 1: cout << "You selected \"Print the array\"\n";
                break;
                case 2: cout << "You selected \"Append element at the end\"\n";
                break;
                case 3: cout << "You selected \"Remove last element\"\n";
                break;
                case 4: cout << "You selected \"Insert one element\"\n";
                break;
                case 5: cout << "You selected \"Exit\"\n";
                        cout << "Exiting program";
                break;
                default: cout << "\nInvalid choice\n";
                                while(choice >= 1 && choice <= 5){
                                        cout << "Please enter a number from 1 to 5: ";
                                        cin >> choice;
                                }
        }
	
		system("pause");
        return 0;
}
