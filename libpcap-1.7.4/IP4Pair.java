import java.util.*;
public class IP4Pair {
    //byte[] sourceArray, destArray;
    int sourceArray, destArray;
    int count;

    public IP4Pair(int s, int d) {
        sourceArray = s;
        destArray = d;
        count = 0;
    }

    public void setSource(int source) {
        sourceArray = source;
    }

    public void setDest(int dest) {
        destArray = dest;
    }

    public void setCount(int c) {
        count = c;
    }

    public int  getcount(){
        return count;
    }
    public byte[] tobyte(int i){
        byte[] ret = new byte[4];
        ret[0] = (byte) (i >>>24);
        ret[1] = (byte) (i >>> 16);
        ret[2] = (byte) (i >>> 8); 
        ret[3] = (byte) (i >>> 0);
        
        return ret;  

    }
    public String printbytearray(byte[] ba){
        String ret = "";
        for(int i = 0; i<4;i++){
        int k = 0xFF;
        k = k & ba[i];
        //System.out.print(ba[i]);
        ret += k + ".";

        }
        //Syste.out.println();
        return ret.substring(0,ret.length() - 1);
    }
    public void incrementcount(){
        if(this.count == 0){
            this.count = 2;
        }else
        this.count ++;
    }
    public boolean match(IP4Pair other) {
        System.out.println("This: " + this.printbytearray(this.tobyte(sourceArray)) + "      " + this.printbytearray(this.tobyte(destArray)));
       System.out.println("Other: " + other.printbytearray(other.tobyte(other.destArray)) + "      " + other.printbytearray(other.tobyte(other.sourceArray)));
       System.out.println("inIP4PAIRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR");
       System.out.println("source match " + Arrays.equals(this.tobyte(sourceArray),other.tobyte(other.destArray)));
      if(Arrays.equals(this.tobyte(sourceArray),other.tobyte(other.destArray))){
           System.out.println("MATCHHHHHHHHH");
           
           return true;
       } 
       //if(other.sourceArray.equals(sourceArray) &&
        //        other.destArray.equals(destArray)) {
       // if(other.sourceArray == sourceArray && 
         //       other.destArray == destArray){
           // count ++;

            //System.out.println("MATHHHHHHHHH");
            //return true;
        //}
        System.out.println("No match");
        return false;
    }

//    public boolean match_array(IP4Pair other) {
  //      Sytem.out.println("This arrays: 

    @Override
    public boolean equals(Object object){
        
        boolean same = false;
        if(object != null && object instanceof IP4Pair){
         IP4Pair obj = (IP4Pair) object;
        same = Arrays.equals(this.tobyte(sourceArray), obj.tobyte(obj.sourceArray)) && Arrays.equals(this.tobyte(destArray), obj.tobyte(obj.destArray));
        
        }
        return same;
    }
}

