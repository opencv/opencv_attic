//////////////////////////////////////////////////////////// 
// uchar cvBackgroundDiff( uchar *p, codeBook &c, 
//                         int minMod, int maxMod)
// Given a pixel and a code book, determine if the pixel is 
// covered by the codebook
//
// p            Pixel pointer (YUV interleaved)
// c            Codebook reference
// numChannels  Number of channels we are testing
// maxMod       Add this (possibly negative) number onto 
//              max level when determining if new pixel is foreground
// minMod       Subract this (possible negative) number from 
//              min level when determining if pixel is foreground
//
// NOTES: 
// minMod and maxMod must have length numChannels, 
// e.g. 3 channels => minMod[3], maxMod[3].
//
// Return
// 0 => background, 255 => foreground
uchar cvBackgroundDiff( uchar *p, codeBook &c, int numChannels, 
                        int *minMod, int *maxMod){
   int matchChannel;
   //SEE IF THIS FITS AN EXISTING CODEWORD
   for(int i=0; i<c.numEntries; i++){
      matchChannel = 0;
      for(int n=0; n<numChannels; n++){
         if((c.cb[i]->min[n] - minMod[n] <= *(p+n)) && 
            (*(p+n) <= c.cb[i]->max[n] + maxMod[n])) 
         {
            matchChannel++; //Found an entry for this channel
         }
         else
         {
            break;
         }
      }
      if(matchChannel == numChannels)
      {
         break; //Found an entry that matched all channels
      }
   }
   if(i >= c.numEntries) return(255);
   return(0);
}
