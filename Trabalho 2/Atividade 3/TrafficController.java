public class TrafficController {
    public Boolean wantRed, wantBlue;
    public long gateA, gateB;

    public TrafficController(){
        gateA = 0;
        gateB = 0;
        wantBlue = false;
        wantRed = false;
    }
    // Wait(1) Esquerda
    public void enterRed(long ID){
        try{Thread.sleep(1);} catch (Exception e){}
        while(true){
            wantRed = true;
            gateA = ID;
            if(gateB != 0 && gateB != ID){
                wantRed = false;
                continue;
            }
            gateB = ID;
            if(gateA != ID){
                wantRed = false;
                while(wantBlue);
                if(gateB != ID) continue;
                else{
                    wantRed = true;
                    break;
                }
            }else break;
        
        }
    }
    // Wait(2) Direita
    public void enterBlue(long ID){
        while(true){
            try{Thread.sleep(1);} catch (Exception e){}
            gateA = ID;
            wantBlue = true;
            if(gateB != 0){
                wantBlue = false;
                continue;
            }
            gateB = ID;
            if(gateA != ID){
                wantBlue = false;
                while(wantRed);
                if(gateB != ID) continue;
                else{
                    wantBlue = true;
                    break;
                }
            }else break;
        }
    }
    // Signal Esquerda
    public void leaveRed(){
        gateB = 0;
        wantRed = false;
    }
    // Signal Direita
    public void leaveBlue(){
        gateB = 0;
        wantBlue = false;
    }

}