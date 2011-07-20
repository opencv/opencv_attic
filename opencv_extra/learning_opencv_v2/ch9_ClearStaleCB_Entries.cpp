///////////////////////////////////////////////////////////////////
//int cvClearStaleEntries(codeBook &c)
// During learning, after you've learned for some period of time, 
// periodically call this to clear out stale codebook entries
//
// c   Codebook to clean up
//
// Return
// number of entries cleared
int cvClearStaleEntries(codeBook &c){
   int staleThresh = c.t>>1; 
   int *keep = new int [c.numEntries];
   int keepCnt = 0;
   //SEE WHICH CODEBOOK ENTRIES ARE TOO STALE
   for(int i=0; i<c.numEntries; i++){
      if(c.cb[i]->stale > staleThresh)
         keep[i] = 0; //Mark for destruction
      else
      {
         keep[i] = 1; //Mark to keep
         keepCnt += 1;
      }
   }
   //KEEP ONLY THE GOOD
   c.t = 0;         //Full reset on stale tracking
   code_element **foo = new code_element* [keepCnt];
   int k=0;
   for(int ii=0; ii<c.numEntries; ii++){
      if(keep[ii])
      {
         foo[k] = c.cb[ii];       
         //We have to refresh these entries for next clearStale
         foo[k]->t_last_update = 0;
         k++;
      }
   }
   //CLEAN UP
   delete [] keep;   
   delete [] c.cb;
   c.cb = foo;
   int numCleared = c.numEntries - keepCnt;
   c.numEntries = keepCnt;
   return(numCleared);
}
