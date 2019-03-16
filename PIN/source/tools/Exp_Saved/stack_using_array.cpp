#include<iostream>
#define MAX 100
 
using namespace std;

char STACK[MAX];
int TOP;
 
//stack initialization
void initStack()
{
    TOP = -1;
}
//check it is empty or not
bool isEmpty()
{
    if(TOP==-1)
        return true;
    else
        return false;
}
 
//check stack is full or not
int isFull()
{
    if(TOP==MAX-1)
        return 1;
    else   
        return 0;
}
 
void push(char num)
{
    if(isFull())
    {
        //cout<<"STACK is FULL."<<endl;
        return; 
    }
    ++TOP;
    STACK[TOP]=num;
    //cout<<num<<" has been inserted."<<endl;
}

//pop - to remove item
void pop(){
    char temp;
    if(isEmpty()){
        //cout<<"STACK is EMPTY."<<endl;
        return;
    }
     
    temp=STACK[TOP];
    TOP--;
    cout<<temp;   
}

void display()
{
    int i;
    if(isEmpty())
    {
        cout<<"STACK is EMPTY."<<endl;
        return;
    }
    for(i=TOP;i>=0;i--){
        cout<<STACK[i]<<" ";
    }
    cout<<endl;
}
 

int main(){
    int num = 0;
    initStack();
    char ch[] = "vishesh";
    while (ch[num] != '\0')
        push(ch[num++]);
    while (!isEmpty())
        pop();
    cout << "\n";
    return 0;
}