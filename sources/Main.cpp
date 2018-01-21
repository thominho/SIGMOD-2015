#include "../headers/Journal.hpp"
#include "../headers/ParserTypes.hpp"
#include "../headers/ListVal.hpp"
#include "../headers/ExtendableHashtable.hpp"
#include "../headers/ListVal2.hpp"
#include "../headers/TridHash.hpp"
#include "../headers/ValidationHash.hpp"
#include "../headers/zombie.hpp"
#include "../headers/ListVal3.hpp"
#include "../headers/JobScheduler.hpp"
#include <sys/time.h>
#include <time.h>

static int processDefineSchema(const DefineSchema& d)
{
   // Insert table column counts into our schema
   schema.clear();
   schema.insert(schema.begin(),d.columnCounts,d.columnCounts+d.relationCount);
   // Resize the relations vector to fit all relations
   relations.clear();
   relations.resize(d.relationCount);
   return d.relationCount;
}



//---------------------------------------------------------------------------
static void processTransaction(const Transaction& t,Journal *d,ExtendableHashtable *h,TridHash *trid,ValidationHash *vh,int a,int options)
{
   const char* reader=t.operations;
   int i=t.transactionId;
   int *array,*array2,size=1,size2=1,currentSize=0,currentSize2=0;
   if(i==0){
	if(a==2){
		size=2;
		size2=100;
	}
	else if(a==30){
		size=2;
		size2=100000;
	}
	else{
		size=2;
		size2=700000;
	}
   }
   else{
   	if(a==2){
		size=100;
		size2=100;
   	}
   	else if(a==30){
		size=8000;
		size2=20000;
	}
	else{
		size=50000;
		size2=500000;
	}
   }
   array=(int *) malloc(sizeof(int)*size);
   array2=(int *) malloc(sizeof(int)*size2);
   // Delete all indicated tuples
   for (uint32_t index=0;index!=t.deleteCount;++index) {
      // Cast operation memory to delete operation and process all row deletes
      auto& o=*reinterpret_cast<const TransactionOperationDelete*>(reader);
      for (const uint64_t* key=o.keys,*keyLimit=key+o.rowCount;key!=keyLimit;++key) {
	 	uint64_t nkey=*key;
		int a=h[o.relationId].findLast(nkey)-1;
		int b=1;
		if(a>=0){
			b=d[o.relationId].return_deletion(a);
		}
		if(b==0)
		{
		array[currentSize]=o.relationId;
		array[currentSize+1]=d[o.relationId].getCurrentPos()+1;
		array[currentSize+2]=nkey;
	  	if(options==1){
	  		int check=trid[o.relationId].find(i);
	 		if(check==0){
	 			//cout << "something2" << endl;
				trid[o.relationId].insert(i,array[currentSize+1]);
	 		}
	 	}	
		d[o.relationId].appendDeletion(i,a);
		currentSize=currentSize+3;
		}
      }
      // Go to the next delete operation
      reader+=sizeof(TransactionOperationDelete)+(sizeof(uint64_t)*o.rowCount);
   }
   // Insert new tuples
   for (uint32_t index=0;index!=t.insertCount;++index) {
      // Cast operation memory to insert operation and process all row inserts
      auto& o=*reinterpret_cast<const TransactionOperationInsert*>(reader);
      for (const uint64_t* values=o.values,*valuesLimit=values+(o.rowCount*schema[o.relationId]);values!=valuesLimit;values+=schema[o.relationId]) {
	  uint64_t a[schema[o.relationId]];
	  for(int j=0;j<schema[o.relationId];j++)
	  {
		a[j]=values[j];
	  }
	  array2[currentSize2]=o.relationId;
	  array2[currentSize2+1]=d[o.relationId].getCurrentPos()+1;
	  array2[currentSize2+2]=a[0];
	  if(options==1){
	  	int check=trid[o.relationId].find(i);
	  	//cout << check << endl;
	 	if(check==0){
	 		//cout << "something" << endl;
			trid[o.relationId].insert(i,array2[currentSize2+1]);
	 	}
	  }
          currentSize2=currentSize2+3;
	  d[o.relationId].appendInsertion(i,a); //insert the records to the correct journal
      }
      // Go to the next insert operation
      reader+=sizeof(TransactionOperationInsert)+(sizeof(uint64_t)*o.rowCount*schema[o.relationId]);
   }
   size=currentSize;
   size2=currentSize2;
   array=(int *) realloc(array,sizeof(int)*size);
   array2=(int *) realloc(array2,sizeof(int)*size2);
   unsigned int offset[2];
   for(int s=0;s<size;s=s+3)//first deletion insertions or just only deletions
   {
	offset[0]=array[s+1];
	offset[1]=0;
	for(int j=0;j<size2;j=j+3)
	{
		if(array[s]==array2[j] && array[s+2]==array2[j+2])
		{
		  offset[1]=array2[j+1];
		  array2[j]=0;
		  array2[j+1]=0;
		  array2[j+2]=0;
		}
	}
	h[array[s]].insert(array[s+2],i,offset);
   }
   for(int s=0;s<size2;s=s+3)//secondly whatever insertion left
   {
	if(array2[s+2]!=0)	
	{
	offset[0]=array2[s+1];
	offset[1]=0;
	h[array2[s]].insert(array2[s+2],i,offset);
	}
   }
   if(array!=NULL)
   {free(array);}
   if(array2!=NULL)
   {free(array2);}
}
//---------------------------------------------------------------------------




static void processValidationQueries(const ValidationQueries& v,List &l,List2 &l2,List3 &l3,ValidationHash *vh,int options)
{
   uint64_t from=v.from,to=v.to;
   int id=v.validationId;
   const char* reader=v.queries; // Pointer to the validation's queries
   // Iterate over the validation's queries
   if(options==1){
	l2.push(id,-1,-1,from,to,NULL);
   }
   else if(options==2){
   	l3.push(id,-1,-1,from,to,NULL);
   }
   else{
   l.push(id,-1,-1,from,to,NULL);
   }
   for (unsigned index=0;index!=v.queryCount;++index) {
      // Cast raw pointer to a query
      auto& q=*reinterpret_cast<const Query*>(reader);
      int k=q.relationId;
      int i=0;
      if(options==1){
		ValidationPredicate *opa;
		for (auto c=q.columns,cLimit=c+q.columnCount;c!=cLimit;++c) {
			uint64_t array[3];
			array[0]=c->column;
			array[1]=c->op;
			array[2]=c->value;
			opa=vh[k].find(from,to,array);
			if(opa==NULL){
				opa=vh[k].insert(from,to,array);
			}
			else{
				opa->in_use_by++;
			}
			if(i==0){
			   l2.push(id,k,-1,from,to,NULL);
			   i++;
			   l2.push(id,-1,i,from,to,opa);
			}
			else{
			   l2.push(id,-1,i,from,to,opa);
			}
			i++;
            	}
		if(i==0){
			l2.push(id,-2,-1,from,to,NULL);
      		}
      }
      else if(options==2){
      	    for (auto c=q.columns,cLimit=c+q.columnCount;c!=cLimit;++c) {
			uint64_t array[3];
			array[0]=c->column;
			array[1]=c->op;
			array[2]=c->value;
			if(i==0){
			   l3.push(id,k,-1,from,to,NULL);
			   i++;
			   l3.push(id,-1,i,from,to,array);
			}
			else{
			   l3.push(id,-1,i,from,to,array);
			}
			i++;
            }
			if(i==0){
			uint64_t array[3];
			array[0]=0;
			array[1]=0;
			array[2]=0;
			l3.push(id,-2,-1,from,to,NULL);
			l3.push(id,-1,i,from,to,array);
			//l3.returnprt(); epistrefei deixth typou val gi auto sou exw ftia3ei constructor deutero gia thn l3 lista pou deineis deixth tupou val
      		}
      		else{
      			//l3.returnprt(); epistrefei deixth typou val gi auto sou exw ftia3ei constructor deutero gia thn l3 lista pou deineis deixth tupou val

      		}
      }
      else{
      		for (auto c=q.columns,cLimit=c+q.columnCount;c!=cLimit;++c) {
			uint64_t array[3];
			array[0]=c->column;
			array[1]=c->op;
			array[2]=c->value;
			if(i==0){
			   l.push(id,k,-1,from,to,NULL);
			   i++;
			   l.push(id,-1,i,from,to,array);
			}
			else{
			   l.push(id,-1,i,from,to,array);
			}
			i++;
            	}
		if(i==0){
			uint64_t array[3];
			array[0]=0;
			array[1]=0;
			array[2]=0;
			l.push(id,-2,-1,from,to,NULL);
			l.push(id,-1,i,from,to,array);
      		}
	}
      // Go to the next query
      reader+=sizeof(Query)+(sizeof(Query::Column)*q.columnCount);
   }
}
//---------------------------------------------------------------------------




//l is for first part,l2 is for second part,l4 is for the third part of project
static void processFlush(const Flush& f,JobScheduler* sch,List &l,List2 &l2,List3 &l4,zombie *l3,Journal *j,ExtendableHashtable *h,TridHash *trid,ValidationHash *vh,int options)
{
	//the check for over the edge of flush is crucial because for loop first check and then increse the i
   if(options==1){
	int k=l2.getfirstvid();
   	int result;
   	for(int i=k;i<=f.validationId;i++){
   		if(i<=f.validationId){
		result=l2.popAt2(j,h,trid,vh,l3);
		cout << result << endl;
		}
   	}
   }
   else if(options==2){//auth h epilogh prokeitai gia ton upologismo mesw nhmatwn
   	int k=l4.getfirstvid();
   	int result;
   	for(int i=k;i<=f.validationId;i++)
   			sch->addJob(popAt2,&l4,j,h,i);

   	sch->barrier();

   	for(int i=k;i<=f.validationId;i++){
   		if(i<=f.validationId){
		result=l4.return_value();
		cout << result << endl;
		}
   	}
   }
   else{
   	int k=l.getfirstvid();
   	int result;
   	for(int i=k;i<=f.validationId;i++){
   		if(i<=f.validationId){
		result=l.popAt2(j,h);
		cout << result << endl;
		}
   	}
   }
}
//---------------------------------------------------------------------------





static void processForget(const Forget& f,ValidationHash *vh,zombie *l3,int a,int options)
{
 if(options==1){
	for(int i=0;i<a;i++){
		int b=10;
		while(b!=0){
			b=l3[i].popAt2(f.transactionId);
			if(b!=0){
				vh[i].remove(b-1);
			}
		}
	}
 }
}
//---------------------------------------------------------------------------




// Read the message body and cast it to the desired type
template<typename Type> static const Type& readBody(istream& in,vector<char>& buffer,uint32_t len) {
      buffer.resize(len);
      in.read(buffer.data(),len);
      return *reinterpret_cast<const Type*>(buffer.data());
}
//---------------------------------------------------------------------------




int main(int argc,char **argv)
{
   vector<char> message;
   Journal *d;
   List l;
   List2 l2;
   List3 l4;
   zombie *l3;
   ExtendableHashtable *h;
   TridHash *trid;
   ValidationHash *vh;
   JobScheduler* sch;
   int options=0;
   int a;
   if(argc>1)
   {
    options=atoi(argv[1]);
    if(options==1)
    {
     cout << "You just enabled the validation hashtable and journal hashtable" << endl;
    }
    if(options == 2)
    {
    	if(argc != 3)
    	{
    		cerr << "Invalid arguements given" << endl;
    		return 1;
    	}

    	sch = new JobScheduler(atoi(argv[2]));
    }
   }
   clock_t t;
   double time_taken;
   while (true) {
      // Retrieve the message
      MessageHead head;
      cin.read(reinterpret_cast<char*>(&head),sizeof(head));
      if (!cin) { cerr << "read error" << endl; abort(); } // crude error handling, should never happen

      // And interpret it
      switch (head.type) {
         case MessageHead::Done:
			t= clock()-t;
			cout << endl;
			time_taken=((double)t)/CLOCKS_PER_SEC;
			printf("Finished in about %f seconds. \n",time_taken);
			for(int i=0;i<a;i++)
				{
					d[i].destroyJournal();
				}
				free(d);
				cout << "Journal Freed" << endl;
				delete[] h;
				cout << " Keys Hashtable Freed" << endl;
				if(options==1){
					delete[] trid;
					cout << "TranscactionIds Hashtable Freed" << endl;
					delete[] vh;
					cout << "Validations Hashtable Freed" << endl;
					delete[] l3;
					cout << "Zombie List Freed" << endl;
				}
				else if(options==2){
					delete sch;
					cout << "Scheduler Freed" << endl;
				}
				cout << "DONE" << endl;
				return 0;

         case MessageHead::DefineSchema:
			a=processDefineSchema(readBody<DefineSchema>(cin,message,head.messageLen));
			d=(Journal *) malloc(sizeof(Journal)*a);
			h=new ExtendableHashtable[a];
			for(int i=0;i<a;i++)
			{
				d[i].createJournal(schema[i]);
			}
			if(options==1){
				trid=new TridHash[a];
				vh=new ValidationHash[a];
				l3=new zombie[a];
			}
			break;

         case MessageHead::Transaction:
			t= clock();
			processTransaction(readBody<Transaction>(cin,message,head.messageLen),d,h,trid,vh,a,options);
			break;

         case MessageHead::ValidationQueries:
			processValidationQueries(readBody<ValidationQueries>(cin,message,head.messageLen),l,l2,l4,vh,options);
			break;

         case MessageHead::Flush:
			processFlush(readBody<Flush>(cin,message,head.messageLen),sch,l,l2,l4,l3,d,h,trid,vh,options);
			break;

         case MessageHead::Forget:
			processForget(readBody<Forget>(cin,message,head.messageLen),vh,l3,a,options);
			break;

         default:
			cerr << "malformed message" << endl;
			abort(); // crude error handling, should never happen
      }
   }
}
