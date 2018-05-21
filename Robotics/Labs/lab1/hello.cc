#include <iostream>
double *v;
int count;
int size;

void Initialize() {
	size = 2;
	count = 0;
	v = new double[size];
}

void Finalize() {
	delete v;
}

int main() {
	Initialize();
	int choice;
	double data;

	while(choice!=5){

	std::cout << "Main menu:\n\n"
                << "1. Print the array\n"
                << "2. Append element at the end\n"
                << "3. Remove last element\n"
                << "4. Insert one element\n"
                << "5. Exit\n\n"
                << "Select an option: ";
	
        std::cin >> choice;

	if(std::cin.fail()) {
		std::cin.clear();
		std::cin.ignore();
		std::cout << "\nInvalid choice. Please enter a number from 1 to 5.\n";
	}
	else {

        switch(choice){
                case 1: std::cout << "You selected \"Print the array\"\n";
 	                break;
                case 2: std::cout << "You selected \"Append element at the end\"\n";
        	        break;
                case 3: std::cout << "You selected \"Remove last element\"\n";
                	break;
                case 4: std::cout << "You selected \"Insert one element\"\n";
			//std::cin>>data;
			//v[size]=data;
			//std::cout<<"Data input and saved was " << v[size]<<std::endl;
               		break;
                case 5: std::cout << "You selected \"Exit\"\n";
                	break;
                default: std::cout << "Invalid choice. Please enter a number from 1 to 5.\n";
        }
	}

	std::cout<<std::endl;
	
	}

	Finalize();
}



