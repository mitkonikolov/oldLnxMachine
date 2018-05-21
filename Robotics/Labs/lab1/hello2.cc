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


void Grow() {
	double *nv = new double[size*2];
	//v[(2*size)-1] = 3.1;
	for(int i=0; i<size; i++) {
		nv[i] = v[i];
	}	

	size = size*2;
	Finalize();
	v=&nv[0];
	std::cout<<"Vector grown"<<std::endl<< 
		"Previous capacity: "<< size/2 << " elements"<< std::endl<<
		"New capacity: " << size <<" elements\n";
}

void Shrink() {
        double *nv = new double[size/2];
        for(int i=0; i<count; i++) {
                nv[i]=v[i];
        }

        Finalize();
        v=&nv[0];
        std::cout<<"Vector shrunk"<<std::endl<<
                "Previous capacity: "<< size << " elements"<< std::endl<<
                "New capacity: " << size/2 <<" elements\n";
        size = size/2;
}


void AddElement() {
	double data;
	std::cout<<"Enter the new element: ";
	std::cin>>data;
	if(count==size) {
		Grow();
	}	
	v[count]=data;
	count++;
	
}	

void PrintVector() {
	for(int i=0; i<count; i++) {
		std::cout<<v[i]<<" ";
	}
}

void RemoveElement() {
	if(count==0) {
		std::cout<<"There are no elements to remove\n";
	}
	else {
		v[count-1] = 0;
		count = count-1;
		double temp = (double)count/size;
		if(temp<0.3) {
			Shrink();
		}
	}
}

void InsertElement() {
	double data;
	int index;
	std::cout<<"Enter the index of the new element: ";
	std::cin>>index;
	if(index>=count) {
		std::cout<<"Invalid index. Element will be added "<<
			"at the end of the array.\n";
		AddElement();
	} 
	else {
		std::cout<<"Enter the new element: ";
		std::cin>>data;
		if(count==size) {
			Grow();
		}
		int temp = count;
		for(int i=0; i<(count-index);i++) {
			v[temp] = v[temp-1];
			temp = temp-1;
		}
		v[index]=data;
		count++;
	}
}

int main() {
	Initialize();
	int choice;

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
			PrintVector();
 	                break;
                case 2: std::cout << "You selected \"Append element at the end\"\n";
			AddElement();
        	        break;
                case 3: std::cout << "You selected \"Remove last element\"\n";
			RemoveElement();
                	break;
                case 4: std::cout << "You selected \"Insert one element\"\n";
			//std::cin>>data;
			//v[size]=data;
			InsertElement();
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

