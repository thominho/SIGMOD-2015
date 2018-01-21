#include "../headers/ListVal.hpp"


	List::val::val (int a,int b,int c){
		vid=a;
		from=b;
		to=c;
		exp_size=0;
		next=NULL;
		right=NULL;
	}

	int List::getfirstvid(){
		if(_size==0){
			return 0;
		}
		return _start->vid;
	}
	
	List::exp::exp(int a,exp* n){
		rid=a;
		size=0;
		next=n;
		query=NULL;
	}
	
	List::queries::queries(uint64_t *a,queries* n){
		query[0]=a[0];
		query[1]=a[1];
		query[2]=a[2];
		next=n;
	}
	
	
	
	List::List (){
		_size=0;
		_start=NULL;
	}

	List::~List (){
		while(_size!=0){
			popAt();
		}
		cout << "Validation List Freed" << endl;
	}

	int List::size () { return _size; }

	int List::push (int vid2, int rid2, int pos, int from, int to, uint64_t *query=NULL)
	{
		val* t=_start;
		if(_size==0){
			_start=new val(vid2,from,to);
			_size++;
		}
		else if(rid2==-1 && pos==-1)/*new validation init*/
		{
			while(t->next!=NULL){
				t=t->next;
			}
			t->next=new val(vid2,from,to);
			_size++;
		}
		else if(rid2!=-1 && pos==-1){/*new tupple init*/
			while(t->vid!=vid2){
				t=t->next;
			}
			t->right=new exp(rid2,t->right);
			t->exp_size++;
		}
		else if(rid2==-1 && pos!=-1){/*new query insert*/
			while(t->vid!=vid2){
				t=t->next;
		 	}
			t->right->query=new queries(query,t->right->query);
			t->right->size++;
		}
	}
//Oi shnuhkes pane: 0(=) 1(!=) 2(<) 3(=<) 4(>) 5(>=)
	int List::popAt2 (Journal *j,ExtendableHashtable *h)//that fuction not only delete the validation we just checked but also run the queries
	{
		if(_size==0){
			return 0;
		}
		val* t = _start;
		_start = _start->next;
		int result_or=0;
		int to,from;
		to=t->to;
		from=t->from;
		while(t->exp_size!=0){
			exp *t2=t->right;
			uint64_t *temp;
			temp=(uint64_t *) malloc(sizeof(uint64_t)*(3*t2->size) );
			int current=0;
			int result_and=1;
			while(t2->size!=0){
				queries *t3=t2->query;
				t2->query=t2->query->next;
				if(t2->rid!=-2){
					temp[current]=t3->query[0];
					temp[current+1]=t3->query[1];
					temp[current+2]=t3->query[2];
					current=current+3;
				}
				delete t3;
				t2->size--;
			}
			if(result_and==1 && t2->rid!=-2){
				result_and=j[t2->rid].valid(temp,current,t2->rid,from,to,h);
			}
			t->right=t->right->next;
			delete t2;
			t->exp_size--;
			if(result_and==1 && result_or==0){
				result_or=1;
			}
			free(temp);
		}
		delete t;
		_size--;
		return result_or;
	}




	bool List::popAt ()
	{
		val* t = _start;
		_start = _start->next;
		while(t->exp_size!=0){
			exp *t2=t->right;
			while(t2->size!=0){
				queries *t3=t2->query;
				t2->query=t2->query->next;
				delete t3;
				t2->size--;
			}
			t->right=t->right->next;
			delete t2;
			t->exp_size--;
		}
		delete t;
		_size--;
		return true;
	}
	
	void List::printList(){
		val *t=_start;
		while(t!=NULL){
			cout << "validation " << t->vid << " " << t->from << "-" << t->to;
			exp *t2=t->right;
			while(t2!=NULL){
				if(t2->rid!=-2){
				cout << " [" << t2->rid << " ";
				}
				queries *t3=t2->query;
				while(t3!=NULL){
					if(t2->rid!=-2){
					cout << "C" << t3->query[0];
					if(t3->query[1]==0){
						cout << "=";
					}
					else if(t3->query[1]==1){
						cout << "!=";
					}
					else if(t3->query[1]==2){
						cout << "<";
					}
					else if(t3->query[1]==3){
						cout << "<=";
					}
					else if(t3->query[1]==4){
						cout << ">";
					}
					else if(t3->query[1]==5){
						cout << ">=";
					}
					}
					if(t2->rid!=-2){
					cout << t3->query[2] << " ";
					}
					t3=t3->next;
				}
				if(t2->rid!=-2){
				cout << "]" << " ";
				}
				t2=t2->next;
			}
			cout << endl;
			t=t->next;
		}
	}
