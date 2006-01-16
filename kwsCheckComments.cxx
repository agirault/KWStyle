/*=========================================================================

  Program:   ITKXML
  Module:    kwsCheckComments.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Author:    Julien Jomier

  Copyright (c) 2002 CADDLab @ UNC. All rights reserved.
  See itkUNCCopyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "kwsParser.h"

namespace kws {

/** Check the comments
 * The comment definition should be set before CheckIndent() to get the correct indentation
 * for the comments. */
bool Parser::CheckComments(const char* begin,const char* middle,const char* end,bool allowEmptyLineBeforeClass)
{
  m_TestsDone[WRONGCOMMENT] = true;
  m_TestsDescription[WRONGCOMMENT] = "The comments are misspelled";
  
  bool hasError = false;
  // Set the ivars for the indent checking
  m_CommentBegin = begin;
  m_CommentMiddle = middle;
  m_CommentEnd = end;

  // We check if we have duplicate code in the comments
  std::vector<PairType>::const_iterator it = m_CommentPositions.begin();
  while(it != m_CommentPositions.end())
    {
    std::string previousWord = "";
    long int i = (*it).first;
    while(i<(*it).second)
      {    
      // we go to the next space
      while(((m_Buffer[i] != ' ') && (m_Buffer[i] != '\r') && (m_Buffer[i] != '\r')) && i<(*it).second)
        {
        i++;
        }
      bool inWord = true;
      bool first = false;
      std::string word = "";
      while(i<(*it).second && inWord)
        {
        if(m_Buffer[i] != ' ' && m_Buffer[i] != '\r' && m_Buffer[i] != '\n')
          {
          word += m_Buffer[i];
          inWord = true;
          first = true;
          }
        else // we have a space
          {
          if(first)
            {
            inWord = false;
            i--;
            }
          }
        i++;
        }

      if(word.size()>0)
        {
        if(word != previousWord)
          {
          previousWord = word;
          }
        else if(previousWord.size() > 1 && 
             previousWord[0] != '/'
             ) // avoid single char and comment
          {
          Error error;
          error.line = this->GetLineNumber(i,false);
          error.line2 = error.line;
          error.number = WRONGCOMMENT;
          error.description = "Duplicate word";
          m_ErrorList.push_back(error);
          hasError = true;
          }
        }
      }
    it++;
    } // end buffer loop

  // Check if there is a comment before each class
  m_TestsDone[MISSINGCOMMENT] = true;
  m_TestsDescription[MISSINGCOMMENT] = "The class should have previously define comments starting with \\class";

  if(allowEmptyLineBeforeClass)
    {
    m_TestsDescription[MISSINGCOMMENT] += " (allowing empty line)";
    }
  else
    {
    m_TestsDescription[MISSINGCOMMENT] += " (not allowing empty line)";
    }

 long int pos = this->GetClassPosition(0);
 while(pos  != -1)
   {
   long int poswithcom = this->GetPositionWithComments(pos);

   // Find the last comment (remove spaces if any)
   std::string commentEnd = "";
   for(unsigned long j=0;j<m_CommentEnd.size();j++)
     {
     if(m_CommentEnd[j] != ' ')
       {
       commentEnd += m_CommentEnd[j];
       }
     }

   long int poscom = m_Buffer.find(commentEnd,0);
   long int poscomtemp = poscom;
   while(poscomtemp!=-1 && poscomtemp<poswithcom)
     {
     poscom = poscomtemp;
     poscomtemp = m_Buffer.find(commentEnd,poscomtemp+1);
     }


   // if we don't have the comment
   if((poscom == -1) || (poscom > poswithcom))
     {
     Error error;
     error.line = this->GetLineNumber(poswithcom,false);
     error.line2 = error.line;
     error.number = MISSINGCOMMENT;
     error.description = "Comment is missing for the class";
     m_ErrorList.push_back(error);
     hasError = true;
     }
   else
     {
     // We check if we have m_CommentEnd before an empty line
     bool emptyLine = false;
     bool gotchar = true;
     for(long int i=poscom;i<poswithcom;i++)
       {
       if(m_Buffer[i] == '\n')
         {
         if(!gotchar)
           {
           emptyLine = true;
           break;
           }
         gotchar = false;
         }

       if( (m_Buffer[i] != '\n') 
         && (m_Buffer[i] != '\r') 
         && (m_Buffer[i] != ' '))
         {
         gotchar = true;
         }   
       }

     if(emptyLine)
       {
       if(!allowEmptyLineBeforeClass)
         {
         Error error;
         error.line = this->GetLineNumber(poswithcom,false);
         error.line2 = error.line;
         error.number = MISSINGCOMMENT;
         error.description = "Comment is missing for the class";
         m_ErrorList.push_back(error);
         hasError = true;
         }
       }
     else  // we check that the word \class exists
       {
       // Find the last 
       long int poscombeg = m_Buffer.find(m_CommentBegin,0);
       long int poscombegt = poscombeg;
       while(poscombegt!=-1 && poscombegt<poscom)
         {
         poscombeg = poscombegt;
         poscombegt = m_Buffer.find(m_CommentBegin,poscombegt+1);
         }

       std::string sub = m_Buffer.substr(poscombeg,poscom-poscombeg);
       if(sub.find("\\class") == -1)
         {
         Error error;
         error.line = this->GetLineNumber(poswithcom,false);
         error.line2 = error.line;
         error.number = MISSINGCOMMENT;
         error.description = "comment doesn't have \\class";
         m_ErrorList.push_back(error);
         hasError = true;
         }
       }
     }
   pos = this->GetClassPosition(pos+1);
   }

  return !hasError;
}

} // end namespace kws
