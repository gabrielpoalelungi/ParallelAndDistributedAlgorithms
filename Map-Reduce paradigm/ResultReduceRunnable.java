public class ResultReduceRunnable implements Comparable{
    String fileName;
    float result;
    int maxSize;
    int numberOfWordsWithMaxSize;
    
    ResultReduceRunnable(String fileName, float result, int maxSize, int numberOfWordsWithMaxSize) {
        this.fileName = fileName;
        this.result = result;
        this.maxSize = maxSize;
        this.numberOfWordsWithMaxSize = numberOfWordsWithMaxSize;
    }

    public int compareTo(Object obj) {
        if (this.result > ((ResultReduceRunnable)obj).result) {
            return -1;
        } else if (this.result == ((ResultReduceRunnable)obj).result){
            return 0;
        } else {
            return 1;
        }
    }
}
