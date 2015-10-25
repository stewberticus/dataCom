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

    public boolean match(IP4Pair other) {
        System.out.println("This: " + sourceArray + "      " + destArray);
        System.out.println("Other: " + other.sourceArray + 
                "      " + other.destArray);
        //if(other.sourceArray.equals(sourceArray) &&
        //        other.destArray.equals(destArray)) {
        if(other.sourceArray == sourceArray && 
                other.destArray == destArray){
            count ++;

            System.out.println("MATHHHHHHHHH");
            return true;
        }
        System.out.println("No match");
        return false;
    }

}

