#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <cstdlib>

using namespace std;

int main(int argc,char**argv)
{
  vector<double>weights; //weights是一个元素类型为double的vector
  vector<string>filenames; //filenames是一个元素类型为string的vector
  for(int i=1;i<argc;++i) //i从1开始，意味着我们只要后面的参数，而不要我们的执行文件名这个参数(argv[0])
    if(string(argv[i])=="-weight")
      weights.push_back(atof(argv[++i]));
    else
      filenames.push_back(argv[i]);//这里是我们正常输入参数时的逻辑，即把我们输入的文件名字push到filenames中去
  
  if((filenames.size()%2)==1||filenames.size()==0 )//这里也可以先不用管，它不对应我们正常输入时的情况
    {
      cerr << argv[0] << " txt1 txt2 [txt3 txt4 -weight w]\n";
      cerr << " Converts plain text into GIZA++ snt-format.\n";
      exit(1);
    }
  //这里定义了很多变量
  string line1,line2,word;
  map<string,int> v1,v2; //词频对应表
  map<string,int> id1,id2; //词id对应表
  vector<string> iid1(2),iid2(2);
  
  string w1(filenames[0]); //这里把我们输入的第一个文件名赋予string w1,比如source.txt
  string w2(filenames[1]); //这里把我们输入的第二个文件名赋予string w2,比如target.txt
   
  //下面这个if做的处理是如果我们的输入文件名w1,w2有.txt,.tok的后缀，我们把这些后缀去掉
  if( w1.length()>4&&w2.length()>4&&((w1.substr(w1.length()-4,w1.length())==".tok" && w2.substr(w2.length()-4,w2.length())==".tok" )||
				     (w1.substr(w1.length()-4,w1.length())==".txt" && w2.substr(w2.length()-4,w2.length())==".txt" ) ))
    {
      w1=w1.substr(0,w1.length()-4);
      w2=w2.substr(0,w2.length()-4);
      cerr << "w1:"<< w1 << " w2:" << w2 << endl;
    } 
      

  string vocab1(w1),vocab2(w2),snt1,snt2; //我们分别用w1,w2去初始化vocab1,vocab2，即vocab1,vocab2都是去掉后缀的形式
  unsigned int slashpos=vocab1.rfind('/')+1; //这里是找到Find last occurrence of content('/') in string,
  //这里的意思是把前面的路径名和文件名分隔开，让我们的slashpos执行文件名的第一个字符的位置
  if( slashpos>=vocab1.length() ) slashpos=0; //这是对应与我们在vocab1中没有找到'/'的情况,返回npos,npos is a static member constant value with the greatest possible value for an element of type size_t
  string vocab1x(vocab1.substr(slashpos,vocab1.length())); //这里是把去掉路径名的vocab1赋予vocab1x
  cout << vocab1 << " -> " << vocab1x << endl;
  //下面是对vocab2做同样的处理
  slashpos=vocab2.rfind('/')+1;
  if( slashpos>=vocab2.length() ) slashpos=0;
  string vocab2x(vocab2.substr(slashpos,vocab2.length())); //同样把去掉路径名的vocab2赋予vocab2x
  cout << vocab2 << " -> " << vocab2x << endl; 
  //注意下面我们有的位置用vocab1(有路径),有的位置用vocab1x(无路径)，道理很简单[对于vocab2,vocab2x也是同理]，是为了在创建文件时保留路径，创建在同一文件夹下面
  snt1=vocab1+"_"+vocab2x+string(".snt"); //给snt1赋予这种格式的值
  snt2=vocab2+"_"+vocab1x+string(".snt"); //给snt2赋予这种格式的值，这里终于解开了我一直以来的疑惑，为什么snt文件的名字的格式那么规整
  vocab1+=string(".vcb"); 
  vocab2+=string(".vcb");
  

  ofstream ovocab1(vocab1.c_str()),ovocab2(vocab2.c_str()),osnt1(snt1.c_str()),osnt2(snt2.c_str()); //根据文件名创建相应的文件
  //下面是从我们的一开始作为参数的文件名对应的文件中读取数据
  for(unsigned int i=0;i<filenames.size();i+=2)
    {
      ifstream i1(filenames[i].c_str()),i2(filenames[i+1].c_str());//读
	  
      if(!i1)cerr << "WARNING: " << filenames[i] << " cannot be read.\n";
      if(!i2)cerr << "WARNING: " << filenames[i+1] << " cannot be read.\n";
	  
      while(getline(i1,line1) && getline(i2,line2) ) //当都能读出一行时(所以我们要求语料文件是一一对应的格式)，我们是对两个文件一行一行的处理
	{
/*
  string line1,line2,word;
  map<string,int> v1,v2;
  map<string,int> id1,id2;
  vector<string> iid1(2),iid2(2);
*/
	  vector<string> t1,t2;
	  istringstream ii1(line1); //把读出行line1和一个istringstream对象绑定起来
	  while(ii1>>word)
	    {
	      t1.push_back(word); //把我们从第一个参数对应的文件中读出的每一行的每一个word push到我们的t1中
	      v1[word]++; //这里是count该word的频数
	      if( id1.find(word)==id1.end() )
		{
		  iid1.push_back(word); //把我们的word push进iid1中，这里注意我们的iid1原始长度是2.
		  id1[word]=iid1.size()-1; //所以当我push第一个word进去时，我们的id是2+1-1=2
		}
	    }
	  //同理于上面的ii1
	  istringstream ii2(line2);
	  while(ii2>>word)
	    {
	      t2.push_back(word);
	      v2[word]++;
	      if( id2.find(word)==id2.end() )
		{
		  iid2.push_back(word);
		  id2[word]=iid2.size()-1;
		}
	    }
	  double w=1.0;
	  if( i/2<weights.size() ) //这个if也先不用去管，因为我们的正常流程中没有输入-weight参数，所以weights.size()==0
	    w=weights[i/2];
	  if( t1.size()&&t2.size() ) //如果t1,t2都非空，这里是向osnt1,onst2中输入
	    {
	      osnt1 << w << "\n"; //所以我们看到每次pair频数都是1，因为之前的那个if block没有执行
	      for(unsigned int j=0;j<t1.size();++j)osnt1 << id1[t1[j]] << ' ';
	      osnt1 << '\n';
	      for(unsigned int j=0;j<t2.size();++j)osnt1 << id2[t2[j]] << ' ';
	      osnt1 << '\n';

	      osnt2 << w << "\n";
	      for(unsigned int j=0;j<t2.size();++j)osnt2 << id2[t2[j]] << ' ';
	      osnt2 << '\n';
	      for(unsigned int j=0;j<t1.size();++j)osnt2 << id1[t1[j]] << ' ';
	      osnt2 << '\n';
	    }
	  else
	    cerr << "WARNING: filtered out empty sentence (source: " << filenames[i] << " " << t1.size() <<
	      " target: " << filenames[i+1] << " " << t2.size() << ").\n";
	}
    }
  //下面是向我们的两个vcb文件中输入
  for(unsigned int i=2;i<iid1.size();++i) //可以看到为什么我们的id是从2开始
    ovocab1 << i << ' ' << iid1[i] << ' ' << v1[iid1[i]] << '\n';
  for(unsigned int i=2;i<iid2.size();++i)
    ovocab2 << i << ' ' << iid2[i] << ' ' << v2[iid2[i]] << '\n';
}
