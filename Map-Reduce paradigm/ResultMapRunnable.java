import java.util.HashMap;
import java.util.List;

public class ResultMapRunnable {
    private String fileName;
    private HashMap<Integer, Integer> dictionary;
    private List<String> maxLengthWords;

    ResultMapRunnable(String fileName, HashMap<Integer, Integer> dictionary, List<String> maxLengthWords) {
        this.fileName = fileName;
        this.dictionary = dictionary;
        this.maxLengthWords = maxLengthWords;
    }

    public String getFileName() {
        return fileName;
    }

    public HashMap<Integer, Integer> getDictionary() {
        return dictionary;
    }

    public List<String> getMaxLengthWords() {
        return maxLengthWords;
    }

    public String toString() {
        return dictionary.toString() + " " + maxLengthWords.toString() + "\n";
    }
}