void dumpLadder(){
  FILE *fp;

  Int_t RunNumber=0;
  Char_t runNumString[20];                                              //temp string for getting run number
  Char_t CurrentRunFileName[200];
  Char_t ScalerFileName[200];
 
  strcpy(CurrentRunFileName,gAR->GetFileName());                        //save curent file name
  strcpy(runNumString,strrchr(CurrentRunFileName,'_')+1);               //copy file name after last "_"
  strcpy(strstr(runNumString,".dat")," ");                              //replace ".dat" with " "
  sscanf(runNumString,"%d",&RunNumber);                                 //scan the run number
  sprintf(ScalerFileName,"/home/edoardo/acqu/acqu_user/scalerdump/scaler%d.dat",RunNumber);           	//construct outfile names

  TH1F *hist = (TH1F*)gDirectory->Get("FPD_ScalerAcc");
  
  fp = fopen(ScalerFileName,"w");

  for(int n = 1; n<=328; n++){
    fprintf(fp,"%d %4.2f\n", n, hist->GetBinContent(n)); 
  }
  fclose(fp);
}
