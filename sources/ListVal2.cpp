#include "../headers/ListVal2.hpp"


	List2::val::val (int a,int b,int c){
		vid=a;
		exp_size=0;
		next=NULL;
		right=NULL;
	}

	int List2::getfirstvid(){
		if(_size==0){
			return 0;
		}
		return _start->vid;
	}
	
	List2::exp::exp(int a,exp* n){
		rid=a;
		size=0;
		next=n;
		query=NULL;
	}
	
	List2::queries::queries(ValidationPredicate *a,queries* n){
		query=a;
		next=n;
	}

	
	
	List2::List2 (){
		_size=0;
		_start=NULL;
	}

	List2::~List2 (){
		while(_size!=0){
			popAt();
		}
		cout << "Validation List Freed" << endl;
	}


	int List2::push (int vid2, int rid2, int pos, int from, int to, ValidationPredicate *query=NULL)
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
//Na ftia3w to pws 8a ginontai ta malloc gia ta apotelesmata wste na ginetai swsta to and kai or
	int List2::popAt2 (Journal *j,ExtendableHashtable *h,TridHash *tid,ValidationHash *vh,zombie *l3)
	{
		if(_size==0){
			return 0;
		}
		val* t = _start;
		_start = _start->next;
		if(t->right->rid==-2){//auto einai gia to adeio tupple
			while(t->exp_size!=0){
					exp *t2=t->right;
					delete t2;
					t->exp_size--;
			}
			delete t;
			_size--;
			return 1;
		}
		uint8_t result_or=0;
		while(t->exp_size!=0){
			exp *t2=t->right;
			uint8_t* result_and;
			uint8_t* temp4;
			if(t2->rid!=-2){
			int search=t2->query->query->range_from;
			int search_by=tid[t2->rid].find(search)-1;
			while(search_by<0 && search<=t2->query->query->range_to){//katw fragma egrafwn pou summetexoun
				search++;
				search_by=tid[t2->rid].find(search)-1;
			}
			int size=0;
			int regs=0;
			if(search_by==-1){
				size=1;
			}
			else{ 
				regs=j[t2->rid].return_reg(search_by,t2->query->query->range_to);
			}
			if(regs%8==0){//should never size be zero
				size=(regs/8);
			}
			else{
				size=(regs/8);
				size=size+1;
			}
			result_and=(uint8_t*) malloc(sizeof(uint8_t)*size);
			if(search_by!=-1){
				for(int k=0;k<size;k++){
					result_and[k]=0xff;
				}
			}
			while(t2->size!=0){//and calculations
				queries *t3=t2->query;
				t2->query=t2->query->next;
				if(search_by!=-1){
					temp4=t3->query->getValue(j,h,tid,t2->rid,search_by,3,size); //returns the bitmap
					for(int k=0;k<size;k++){
						result_and[k]=result_and[k] & temp4[k];
					}
				}
					t3->query->in_use_by--;
					if(t3->query->in_use_by==0){
						unsigned int a=vh[t2->rid].getOffset(t3->query);
						if(a==0){
							cerr << "predicate was not inside the hashtable:This should not happen" << endl;
							exit(1);
						}
						else{
							l3[t2->rid].push(a,t3->query->range_from);
						}
					}
				t3->query=NULL;
				delete t3;
				t2->size--;
			}
			if(search_by!=-1 && result_or==0){//elenxei an ikanopoiountai se toulaxiston mia eggrafh oles oi proupotheseis
				for(int k=0;k<size;k++){
					if(result_and[k]!=0x00  && result_or==0){//an yparxei mia eggrafh pou teirei tis proupotheseis alla3e to bit se 1
						result_or=1;
					}
				}
				//printf("%luaaaaaa3\n",a);
			}
			free(result_and);
		}
		else{
			result_or=1;
		}
			t->right=t->right->next;
			delete t2;
			t->exp_size--;
		}
		delete t;
		_size--;
		return result_or;
	}

	bool List2::popAt ()
	{
		if(_size==0){
			return 0;
		}
		val* t = _start;
		_start = _start->next;
		while(t->exp_size!=0){
			exp *t2=t->right;
			while(t2->size!=0){
				queries *t3=t2->query;
				t2->query=t2->query->next;
				t3->query=NULL;
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