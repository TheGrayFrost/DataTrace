#include<iostream>

using namespace std;

struct node
{
    int data;
    node * next;
} *front = NULL, *rear = NULL, *p = NULL, *np = NULL;

void push(int x)
{
    np = new node;
    np->data = x;
    np->next = NULL;
    if(front == NULL)
    {
        front = rear = np;
        rear->next = NULL;
    }
    else
    {
        rear->next = np;
        rear = np;
        rear->next = NULL;
    }
}
int pop()
{
    int x;
    if(front == NULL)
        return -1;
    else
    {
        p = front;
        x = p->data;
        front = front->next;
        delete(p);
        return(x);
    }
}

int main()
{
    int n, c = 0, x;
    cout << "Enter the number of values to be pushed into queue\n";
    cin >> n;
    while (c < n)
    {
    	cout<<"Enter the value to be entered into queue\n";
    	cin>>x;
        push(x);
        c++;
     }
     cout<<"\n\nRemoved Values\n\n";
     while(true)
     {
        if (front != NULL)
            cout<<pop()<<endl;
        else
            break;
     }
     getch();
}