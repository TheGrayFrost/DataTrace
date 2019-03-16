#include<iostream>
using namespace std;


//   Creating a NODE Structure
struct node
{
    char data;
    struct node *next;
};

// Creating a class STACK
class stack
{
    struct node *top;
    public:
    stack() // constructor
    {
        top=NULL;
    }
    void push(char); // to insert an element
    void pop();  // to delete an element
    void show(); // to show the stack
    bool isEmpty();
};

bool stack::isEmpty()
{
    if (top == NULL)
        return true;
    return false;
}
// PUSH Operation
void stack::push(char value)
{
    //int value;
    struct node *ptr;
    //cout<<"\nPUSH Operationn";
    //cout<<"Enter a number to insert: ";
    //cin>>value;
    ptr=new node;
    ptr->data=value;
    ptr->next=NULL;
    if(top!=NULL)
        ptr->next=top;
    top=ptr;
    //cout<<"\nNew item is inserted to the stack!!!";
}

// POP Operation
void stack::pop()
{
    struct node *temp;
    if(top==NULL)
    {
        //cout<<"\nThe stack is empty!!!";
    }
    temp=top;
    top=top->next;
    cout<<temp->data;
    delete temp;
}

// Show stack
void stack::show()
{
    struct node *ptr1=top;
    cout<<"\nThe stack is\n";
    while(ptr1!=NULL)
    {
        cout<<ptr1->data<<" ->";
        ptr1=ptr1->next;
    }
    cout<<"NULL\n";
}

// Main function
int main()
{
    stack s;
    int num = 0;
    //initStack();
    char ch[] = "vishesh";
    while (ch[num] != '\0')
        s.push(ch[num++]);
    while (!s.isEmpty())
        s.pop();
    cout << "\n";
    return 0;
}