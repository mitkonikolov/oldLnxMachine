#include <iostream>
using namespace std;
	
	double* p;
    int count;
    int size;

void Initialize() {
    p = new double[2];
    count = 0;
    size = 2;
}

void Finalize() {
    delete p;
}

void Print(){
 	if (count == 0)
 	{
 		cout << "\nThe array is empty\n\n"; cin.get();
	 }
		 
	else{
		cout << "\nArray: \n";
 		for(int i = 0; i < count; i++){
			cout << p[i] << " ";
		}
		cout << endl << endl;
	}
}

 void AddElement(){
 	cout << "\nEnter new element: ";
	cin >> p[count];;
	count++;
	cout << endl;
}


int main(){
    int choice;
    bool exit = false;
    Initialize();

while(!exit){
		cout << "Main menu:\n\n"
                << "1. Print the array\n"
                << "2. Append element at the end\n"
                << "3. Remove last element\n"
                << "4. Insert one element\n""
                << "5. Exit\n\n"
                << "Select an option: ";
        cin >> choice;

        switch(choice){
                case 1: cout << "You selected \"Print the array\"\n";
                		Print();
                		break;
                case 2: cout << "You selected \"Append element at the end\"\n";
                		
                		break;
                case 3: cout << "You selected \"Remove last element\"\n";
                
                		break;
                case 4: cout << "You selected \"Insert one element\"\n";
                		AddElement();
                		break;
                case 5: cout << "You selected \"Exit\"\n\n";
                        cout << "Exiting program\n";
                        exit = true;
						break;
                default: cout << "\t\t\nINVALID CHOICE!\n";	cin.get();                                       
        }
	}
    system("pause");
	return 0;
}
