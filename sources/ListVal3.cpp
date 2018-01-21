#include "../headers/ListVal3.hpp"


	List3::val::val (int a,int b,int c){
		vid=a;
		from=b;
		to=c;
		value=0;
		exp_size=0;
		next=NULL;
		right=NULL;
	}

	int List3::getfirstvid(){
		if(_size==0){
			return 0;
		}
		return _start->vid;
	}
	
	List3::exp::exp(int a,exp* n){
		rid=a;
		size=0;
		next=n;
		query=NULL;
	}
	
	List3::queries::queries(uint64_t *a,queries* n){
		query[0]=a[0];
		query[1]=a[1];
		query[2]=a[2];
		next=n;
	}
	
	
	
	List3::List3 (){
		_size=0;
		_start=NULL;
	}

	List3::List3(val *n){
		_start=n;
	}

	List3::~List3 (){
		while(_size!=0){
			popAt();
		}
		cout << "Validation List Freed" << endl;
	}

	int List3::size () { return _size; }

	int List3::push (int vid2, int rid2, int pos, int from, int to, uint64_t *query=NULL)
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
	int List3::popAt2 (Journal *j,ExtendableHashtable *h,int id)//that fuction not only delete the validation we just checked but also run the queries
	{
		if(_size==0){
			return 0;
		}
		
		val* node = _start;
		/*the data that t is pointing to are accessed only by current thread
		  provided that each thread call the function with different id*/
		while(node!=NULL){
			if(node->vid==id){
				break;
			}
			node=node->next;
		}

		if(node == NULL)
		{
			return 0;
		}

		int result_or = 0;
		int to,from;
		to = node->to;
		from = node->from;
		exp* expIter = node->right;
		while(expIter != NULL){
			uint64_t *temp = (uint64_t *) malloc(sizeof(uint64_t)*(3*expIter->size) );
			if(temp == NULL)
			{
				perror("Failed to allocate temporary memory on popAt2");
			}
			int current=0;
			int result_and=1;
			queries *queriesIter = expIter->query;
			while(queriesIter != NULL){

				if(expIter->rid != -2){
					temp[current] = queriesIter->query[0];
					temp[current+1] = queriesIter->query[1];
					temp[current+2] = queriesIter->query[2];
					current = current + 3;
				}

				queriesIter = queriesIter->next;
			}
			if(result_and == 1 && expIter->rid != -2){
				result_and = j[expIter->rid].valid3(temp,current,expIter->rid,from,to,h);
			}
			expIter = expIter->next;
			free(temp);
			if(result_and == 1){
				result_or = 1;
				break;
			}
		}
		node->value = result_or; 	//cerr << "Node Value: " << node->value << endl;
	}

	void List3::nullifier(){
		_start=NULL;
	}

	int List3::return_value(){
		if(_size==0){
			return 0;
		}
		val* t = _start;
		_start = _start->next;
		int result=t->value;
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
		return result;
	}


	bool List3::popAt ()
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
	
	void List3::printList(){
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


int popAt2(List3* l3,Journal *j,ExtendableHashtable *h,int id)
{
	l3->popAt2(j,h,id);
	return 0;
}