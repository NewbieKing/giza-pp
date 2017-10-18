/*

EGYPT Toolkit for Statistical Machine Translation
Written by Yaser Al-Onaizan, Jan Curin, Michael Jahr, Kevin Knight, John Lafferty, Dan Melamed, David Purdy, Franz Och, Noah Smith, and David Yarowsky.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
USA.

*/
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <set>
#include "defs.h"
#include "vocab.h" 
#include "Perplexity.h"
#include "getSentence.h"
#include "TTables.h"
#include "Globals.h"
#include "Parameter.h"

void printHelp(void)
{
  cerr << "Usage:\n\n" << Usage << '\n';
  cerr << "Options (these override parameters set in the config file):\n\n";
  cerr << "\t--v \t\t print verbose message, Warning this is not very descriptive and not systematic.\n";
  cerr << "\t--NODUMPS \t Do not write any files to disk (This will over write dump frequency options).\n";
  cerr << "\t--h[elp]\t\tprint this help\n";
  cerr << "\t--p\t\tUse pegging when generating alignments for Model3 training.  (Default NO PEGGING)\n";
  cerr << "\t--st\t\tto use a fixed ditribution for the fertility parameters when tranfering from model 2 to model 3 (Default complicated estimation)\n";
  printGIZAPars(cout);
}


void generatePerplexityReport(const Perplexity& trainperp, 
			      const Perplexity& testperp, 
			      const Perplexity& trainVperp, 
			      const Perplexity& testVperp, 
			      ostream& of, int trainsize, int testsize, 
			      bool)
{
  unsigned int i, m;
  unsigned int m1 = max(trainperp.size(), testperp.size());
  unsigned int m2 = max(trainVperp.size(), testVperp.size());
  m = max(m1,m2);
  of << "#trnsz\ttstsz\titer\tmodel\ttrn-pp\t\ttest-pp\t\ttrn-vit-pp\t\ttst-vit-pp\n";
  for (i = 0 ; i <m ; i++){
    of << trainsize << '\t' << testsize << '\t' << i<< '\t' << trainperp.modelid[i] << '\t';
    if (i < trainperp.perp.size())
      of << trainperp.perp[i] << "\t\t" ;
    else 
      of << "N/A\t\t";
    if (i<testperp.perp.size())
      of << testperp.perp[i] << "\t\t" ;
    else 
      of << "N/A\t\t";
    if (i < trainVperp.perp.size())
      of << trainVperp.perp[i] << "\t\t" ;
    else 
      of << "N/A\t";
    if (i< testVperp.perp.size())
      of << testVperp.perp[i] << '\n' ;
    else 
      of << "N/A\n";
  }
}

void  printSentencePair(Vector<WordIndex>& es, 
			Vector<WordIndex>& fs, 
			ostream& of)
  
  // just writes a sentece pair to the give output stream, one sentence pair line
  // it writes token ids not actual tokens.
{
  WordIndex i, j, l, m;
  l = es.size() - 1;
  m = fs.size() - 1;
  of << "Source sentence length : " << l << " , target : " << m << "\n"; 
  for (i = 1 ; i <= l ; i++)
    of << es[i] << ' ';
  of << "\n";
  for (j = 1 ; j <= m ; j++)
    of << fs[j] << ' ';
  of << "\n";

}

extern bool newflag;
extern bool oovflag;
//extern vector<bool> resizeflag_vec;
//extern int cur;
//extern bool dump_flag;
extern short CompactAlignmentFormat;
//该函数是打印一个sentence pair的alignment结果
void printAlignToFile(const Vector<WordIndex>& es, 
		      const Vector<WordIndex>& fs, 
		      const Vector<WordEntry>& evlist, 
		      const Vector<WordEntry>& fvlist, 
		      ostream& of2, 
		      const Vector<WordIndex>& viterbi_alignment, 
		      int pair_no, double alignment_score,string sou_sent,string tar_sent)
     
     // prints the given alignment to alignments file (given it stream pointer)
     // in a format recognizable by the draw-alignment tool ... which is of the
     // example (each line triple is one sentence pair): 
     //   # sentence caption 
     //   target_word_1 target_word_2  ..... target_word_m
     //   source_word_1 ({ x y z }) source_word_2 ({ })  .. source_word_n ({w})
     // where x, y, z, and w are positions of target words that each source word
     // is connected to.

{
  WordIndex l, m;
	
  Vector<Vector<WordIndex> > translations(es.size()); // each english words has a vector 
	
  // of zero or more translations .
  l = es.size() - 1;
  m = fs.size() - 1;	
  if( CompactAlignmentFormat ) //这里CompactAlignmentFormat是我们的main.cpp中定义的全局变量值为0，所以走else
    {
      for (WordIndex j = 1 ; j <= m ; j++)
	if( viterbi_alignment[j] )
	  of2 << viterbi_alignment[j]-1 << ' ' << j-1 << ' ';
      of2 << '\n';
    }
  else //走这个
    {
      if(newflag&&oovflag&&(sou_sent!="")&&(tar_sent!=""))
      {
      vector<string> sou_list,tar_list;
      string word1,word2;
      istringstream sou(sou_sent);
      istringstream tar(tar_sent);
      while(sou>>word1)
      		sou_list.push_back(word1);
      while(tar>>word2)
	      	tar_list.push_back(word2);
      vector<bool> incre_flag_list(tar_list.size(),0);
      vector<bool> tfind_flag_list(m+1,0);
      vector<bool> sfind_flag_list(l+1,0);
      //下面是输出内容
      of2 << "# Sentence pair (" << pair_no <<") source length " << l << " target length "<< m << 
	" alignment score : "<< alignment_score << '\n';
      //要记得es[0],fs[0]都是"NULL"
      //target sentence
      for (WordIndex k = 0 ; k < tar_list.size() ; k++){
	     if(tar_list[k][tar_list[k].size()-1]=='0') //注意这里一定不要忘记我们索引出来的是一个字符！！！所以不能直接和数字0比较
	   //注意这里我们的每个tar_list[k]是有汉语，而一个汉字所占用的不是一个char字符位置
 	     {
		     of2<<tar_list[k]<<"!@#"<<" "; 
	             incre_flag_list[k]=1;
	     }
 	     else
	    {
		int incre=0;
//因为我们之前在处理oov时是直接把oov丢掉，然后把剩下的组成snt传入GIZA++计算，所以这里在把oov word加回来后，对我们的计算的alignment位置会有影响
		for(WordIndex j=1; j<=m ;j++)
		{
			if(fvlist[fs[j]].word+"1"==tar_list[k])
			{
	 			if(tfind_flag_list[j]==0)
				{
					of2 << fvlist[fs[j]].word << " " ;
					for(int p=0;p<k;p++)
						incre+=incre_flag_list[p];
					translations[viterbi_alignment[j]].push_back(j+incre); //计算得到的位置应该是j，而把oov加回来后，偏移为j+incre
					tfind_flag_list[j]=1;
					break;
				}
			}
		}
	    }
      } 
      /*
      if(dump_flag&&resizeflag_vec[cur-1])
		of2<<"the pair has been resized!";
      */
      of2 << '\n';
      
      //source sentence
      of2 << evlist[es[0]].word << " ({ " ;
      for (WordIndex j = 0 ; j < translations[0].size() ; j++)
	  of2 << translations[0][j] << " " ;
      of2 << "}) ";
      for (WordIndex k = 0  ; k < sou_list.size() ; k++){
	if(sou_list[k][sou_list[k].size()-1]=='0')
	{
		of2<<sou_list[k]<<"@!#"<<" ";
	}
	else
	{
		for(WordIndex i=1;i<=l;i++)
		{
			if(evlist[es[i]].word+"1"==sou_list[k])
			{
				if(sfind_flag_list[i]==0)
				{
					of2 << evlist[es[i]].word << " ({ " ;
					for (WordIndex j = 0 ; j < translations[i].size() ; j++)
	  					of2 << translations[i][j] << " " ;
					of2 << "}) ";
					sfind_flag_list[i]=1;
					break;
				}
			}
		}
	}
      }
      /*
      if(dump_flag&&resizeflag_vec[cur-1])
		of2<<"the pair has been resized!";
      */
      of2 << '\n';
      }//我们的if分支的结尾
	  
      else
      {
      of2 << "# Sentence pair (" << pair_no <<") source length " << l << " target length "<< m << 
	" alignment score : "<< alignment_score << '\n';
      //要记得es[0],fs[0]都是"NULL"
      for (WordIndex j = 1 ; j <= m ; j++){
	of2 << fvlist[fs[j]].word << " " ;
	translations[viterbi_alignment[j]].push_back(j);
      }
      of2 << '\n';
      
      for (WordIndex i = 0  ; i <= l ; i++){
	of2 << evlist[es[i]].word << " ({ " ;
	for (WordIndex j = 0 ; j < translations[i].size() ; j++)
	  of2 << translations[i][j] << " " ;
	of2 << "}) ";
      }
      of2 << '\n';
      }//else分支的结尾
    }//更大的else分支的结尾
}//函数的结尾


void printOverlapReport(const tmodel<COUNT, PROB>& tTable, 
			sentenceHandler& testHandler,  vcbList& trainEList, 
			vcbList& trainFList, vcbList& testEList, vcbList& testFList)
{
  set<pair<WordIndex, WordIndex> > testCoocur ;
  sentPair s ;
  /*  string unseenCoocurFile = Prefix + ".tst.unseen.cooc" ;
      ofstream of_unseenCoocur(unseenCoocurFile.c_str());
      
      string seenCoocurFile = Prefix + ".tst.seen.cooc" ;
      ofstream of_seenCoocur(seenCoocurFile.c_str());
  */  
  testHandler.rewind();
  int seen_coocur = 0, unseen_coocur = 0, srcUnk = 0, trgUnk = 0 ;
  while(testHandler.getNextSentence(s)){    
    for (WordIndex i = 1 ; i < s.eSent.size() ; i++)
      for (WordIndex j = 1 ; j < s.fSent.size() ; j++)	
	testCoocur.insert(pair<WordIndex, WordIndex> (s.eSent[i], s.fSent[j])) ;
  }
  set<pair<WordIndex, WordIndex> >::const_iterator i ;
  for (i = testCoocur.begin() ; i != testCoocur.end() ; ++i){
    if (tTable.getProb((*i).first, (*i).second) > PROB_SMOOTH){
      seen_coocur ++ ;
      //      of_seenCoocur << (*i).first << ' ' << (*i).second << '\n';
    }
    else {
      unseen_coocur++;
      //      of_unseenCoocur << (*i).first << ' ' << (*i).second << '\n';
    }
  }
  
  string trgUnkFile = Prefix + ".tst.trg.unk" ;
  ofstream of_trgUnk(trgUnkFile.c_str());

  for (WordIndex i = 0 ; i <  testFList.getVocabList().size() && i < testFList.uniqTokens();i++)
    if (testFList.getVocabList()[i].freq > 0 && trainFList.getVocabList()[i].freq <= 0){
      of_trgUnk << i << ' ' << testFList.getVocabList()[i].word << ' ' << testFList.getVocabList()[i].freq
		<< '\n';
      trgUnk++ ;
    }
  string srcUnkFile = Prefix + ".tst.src.unk" ;
  ofstream of_srcUnk(srcUnkFile.c_str());

  for (WordIndex j = 0 ; j <  testEList.getVocabList().size() && j < testEList.uniqTokens();j++)
    if (testEList.getVocabList()[j].freq > 0 && trainEList.getVocabList()[j].freq <= 0){
      srcUnk++ ;
      of_srcUnk << j << ' ' << testEList.getVocabList()[j].word << ' ' << testEList.getVocabList()[j].freq
		<< '\n';
    }
  string summaryFile = Prefix + ".tst.stats" ;  
  ofstream of_summary(summaryFile.c_str());
  of_summary << "\t\t STATISTICS ABOUT TEST CORPUS\n\n";
  of_summary << "source unique tokens: " <<  testEList.uniqTokens() << '\n';
  of_summary << "target unique tokens: " <<  testFList.uniqTokens() << '\n';
  of_summary << "unique unseen source tokens: " << srcUnk << '\n';
  of_summary << "unique unseen target tokens: " << trgUnk << '\n';
  of_summary << "cooccurrences not found in the final t table: " << unseen_coocur << '\n';
  of_summary << "cooccurrences found in the final t table: " << seen_coocur << '\n';
  
}

