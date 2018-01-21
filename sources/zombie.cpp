#include "../headers/zombie.hpp"

	zombie::val::val(int temp,int temp2,val *n){
		offset=temp;
		from=temp2;
		next=n;
	}

	zombie::zombie (){
		_size=0;
		_start=NULL;
	}

	zombie::~zombie (){
		while(_size!=0){
			popAt();
		}
		cout << "Zombie List Freed" << endl;
	}

	int zombie::size () { return _size; }

	int zombie::push (int temp,int temp2)
	{
		_start=new val(temp,temp2,_start);
		_size++;
	}

	unsigned int zombie::popAt2 (int temp)
	{
		if(_size==0){
			return 0;
		}
		if(_start->from>=temp){
			unsigned int offset;
			val *t=_start;
			offset=t->offset;
			_start=_start->next;
			delete t;
			_size--;
			return offset;
		}
		else{
			val *t=_start;
			val *t2=_start->next;
			while(t2!=NULL){
				if(t2->next==NULL){
					if(t2->from>=temp){
						t->next=NULL;
						int offset=t2->offset;
						delete t2;
						_size--;
						return offset;
					}
				}
				else if(t2->from>=temp){
					val *temp=t2->next;
					t->next=temp;
					int offset=t2->offset;
					delete t2;
					_size--;
					return offset;
				}
				t=t->next;
				t2=t2->next;
			}
		}
		return 0;
	}

	bool zombie::popAt(){
			val *t=_start;
			_start=_start->next;
			delete t;
			_size--;
	}
