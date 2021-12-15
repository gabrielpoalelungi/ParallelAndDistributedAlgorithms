import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ReduceRunnable implements Runnable{
    // current document
    public String fileName;

    // index of documents in the array Files in Tema2 class;
    public int filePosition;

    // this document's list of dictionaries with key = length of word and value = number of appearances;
    public List<HashMap<Integer, Integer>> currentFileDictionary;

    // this document's list of lists of max length words;
    public List<List<String>> currentFileMaxLenWords;

    ReduceRunnable(int filePosition, String fileName, List<HashMap<Integer, Integer>> currentFileDictionary,
        List<List<String>> currentFileMaxLenWords) {
        this.filePosition = filePosition;
        this.fileName = fileName;
        this.currentFileDictionary = currentFileDictionary;
        this.currentFileMaxLenWords = currentFileMaxLenWords;
    }

    // Fibonacci function for computing rank of document
    public int fibonacciFunction(int position) {
        int f1 = 1;
        int f2 = 1;
        int f3 = 2;

        if(position == 2 || position == 1) {
            return 1;
        } else {
            for(int i = 2; i < position; i++) {
                f3 = f2 + f1;
                f1 = f2;
                f2 = f3;
            }
            return f3;
        }
    }

    @Override
    public void run() {
        // Final dictionary for the current document;
        HashMap<Integer, Integer> finalDictionary = new HashMap<>();

        // this document's rank which is to be computed;
        float result;
        // The maximum length for a word in this document;
        int maxSize = 0;

        // How many words there are with maximum length;
        int numberOfWordsWithMaxSize = 0;

        // Combine fragments' dictionaries into a single one which represents the whole doc dictionary;
        for(int i = 0; i < currentFileDictionary.size(); i++) {
            for(Map.Entry<Integer, Integer> set : currentFileDictionary.get(i).entrySet()) {
                if (finalDictionary.containsKey(set.getKey())) {
                    finalDictionary.replace(set.getKey(), finalDictionary.get(set.getKey()) + set.getValue());
                } else {
                    finalDictionary.put(set.getKey(), set.getValue());
                }
                // Find the maximum size of a word;
                if(set.getKey() > maxSize) {
                    maxSize = set.getKey();
                }
            }
        }

        // Count how many words with maximum size are in this list of lists of max length word;
        // These words are going to be the maximum size in the whole document;
        for(int i = 0; i < currentFileMaxLenWords.size(); i++) {
            for(String s : currentFileMaxLenWords.get(i)) {
                if (s.length() == maxSize) {
                    numberOfWordsWithMaxSize++;
                }
            }
        }

        // Calculating the rank;
        int count = 0;
        float sum = 0;
        for(Map.Entry<Integer, Integer> s : finalDictionary.entrySet()) {
            count += s.getValue();
            sum += fibonacciFunction(s.getKey() + 1) * s.getValue();
        }

        result = sum / count;
        // Send the result to main by adding it in an array of Reduce results;
        Tema2.finalResult.add(new ResultReduceRunnable(fileName, result, maxSize, numberOfWordsWithMaxSize));
        Tema2.inQueue.decrementAndGet();
    }
}