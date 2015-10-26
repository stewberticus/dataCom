public class IP4Pair {
    //byte[] sourceArray, destArray;
    int sourceArray, destArray;
    int count;

    public IP4Pair(int s, int d) {
        sourceArray = s;
        destArray = d;
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

    public boolean match(IP4Pair other) {
        System.out.println("This: " + sourceArray + "      " + destArray);
       System.out.println("Other: " + other.destArray + 
               "      " + other.sourceArray);
       System.out.println("inIP4PAIRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR");
       System.out.println("source match " + (destArray - other.sourceArray));
       if(destArray - other.sourceArray == 135){
           System.out.println("MATCHHHHHHHHH");
           count++;
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

}

