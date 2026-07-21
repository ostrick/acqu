void PeriodMacro()
{

    static ULong64_t previousScalerCount = 0;

    TA2PairSpec* pairSpec =
    (TA2PairSpec*)gAN->GetGrandChild("PairSpec", "TA2Detector");

    ULong64_t scalersInPeriod = 0;

    if (pairSpec) {
        ULong64_t currentScalerCount =
        pairSpec->GetScalerEventCount();

        scalersInPeriod =
        currentScalerCount - previousScalerCount;

        previousScalerCount = currentScalerCount;
    }

    printf("Events: %d, akzeptiert: %d, Scaler-Events: %llu\n",
           gAN->GetNEvent(),
           gAN->GetNEvAnalysed(),
           (unsigned long long)scalersInPeriod);


        // Bisheriger PairSpec-Code
        TH1* openHist = (TH1*)gROOT->FindObject("PairSpec_Open");
        TH1* gatedHist = (TH1*)gROOT->FindObject("PairSpec_Gated");
        TH1* delayedHist = (TH1*)gROOT->FindObject("PairSpec_GatedDly");
        TH1* fpdHist = (TH1*)gROOT->FindObject("FPD_ScalerCurr");

        if (openHist && gatedHist && delayedHist && fpdHist &&
            openHist->Integral() > 0) {
            Double_t open = fpdHist->Integral(0, -1);
            Double_t gated = gatedHist->Integral(0, -1);
            Double_t delayed = delayedHist->Integral(0, -1);

        if (open != 0) {
            Double_t taggEff = (gated - delayed) / open * 25.0;

            printf("Sums: o %f  g_s %f  gd_s %f  taggeff_s %f\n",
                   open, gated, delayed, taggEff);
        }
            }
}
