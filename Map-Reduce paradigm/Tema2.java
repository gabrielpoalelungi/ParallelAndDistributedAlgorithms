import java.io.*;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicInteger;

public class Tema2 {

    public static int fragmentDimension;
    public static int numberOfDocuments;
    private static Scanner myScanner;
    private static FileReader inputFile;
    public static FileWriter outputFile;
    public static String[] files;
    public static ExecutorService tpe;
    public static AtomicInteger inQueue = new AtomicInteger(0);
    public static List<ResultMapRunnable> resultsMap = Collections.synchronizedList(new ArrayList<>());
    public static ConcurrentHashMap<Integer, Integer> finalDictionary = new ConcurrentHashMap<>();
    public static List<ResultReduceRunnable> finalResult = Collections.synchronizedList(new ArrayList<ResultReduceRunnable>());
    public static void main(String[] args) {
        if (args.length < 3) {
            System.err.println("Usage: Tema2 <workers> <in_file> <out_file>");
            return;
        }

        try {
            inputFile = new FileReader(args[1]);
            myScanner = new Scanner(inputFile);
            
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }

        try {
            outputFile = new FileWriter(args[2]);
        } catch (IOException e) {
            e.printStackTrace();
        }

        fragmentDimension = Integer.parseInt(myScanner.nextLine());
        numberOfDocuments = Integer.parseInt(myScanner.nextLine());

        // Array of strings which store the filenames of docs who are to be analyzed;
        files = new String[numberOfDocuments];
        for(int i = 0; i < numberOfDocuments; i++) {
            files[i] = myScanner.nextLine();
        }

        tpe = Executors.newFixedThreadPool(Integer.parseInt(args[0]));
         
        for(int i = 0; i < numberOfDocuments; i++) {
            try {
                // Doc to be analyzed;
                RandomAccessFile currentFile = new RandomAccessFile("../" + files[i], "r");
                try {
                    long lengthOfFile = currentFile.length();
                    int offset = 0;

                    // Creating Map tasks for workers;
                    while(offset <= lengthOfFile) {
                        if(lengthOfFile - offset > fragmentDimension) {
                            tpe.submit(new MapRunnable(files[i], offset, fragmentDimension));
                            inQueue.incrementAndGet();
                        } else {
                            tpe.submit(new MapRunnable(files[i], offset, (int)lengthOfFile - offset));
                            inQueue.incrementAndGet();
                        }
                        offset += fragmentDimension;
                    }
                    currentFile.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            }
        }

        // Busy waiting till all the workers are done;
        while(inQueue.get() != 0);

        // Restarting workers;
        tpe.shutdown();        
        tpe = Executors.newFixedThreadPool(Integer.parseInt(args[0]));

        // Creating Reduce tasks for workers;
        for(int i = 0; i < numberOfDocuments; i++) {
            // List of dictionaries for a certain document;
            List<HashMap<Integer, Integer>> currentFileDictionary = new ArrayList<>();

            // List of lists of max length words for each document;
            List<List<String>> currentFileMaxLenWords = new ArrayList<>();

            // For every document, create a list of dictionaries and a list of lists of max length
            // words and create a Reduce task with these + current document filename;
            for(int j = 0; j < resultsMap.size(); j++) {
                if(resultsMap.get(j).getFileName().equals(files[i])) {
                    currentFileDictionary.add(resultsMap.get(j).getDictionary());
                    currentFileMaxLenWords.add(resultsMap.get(j).getMaxLengthWords());
                }
            }
            tpe.submit(new ReduceRunnable(i, files[i], currentFileDictionary, currentFileMaxLenWords));
            inQueue.incrementAndGet();
        }

        // Busy waiting till all the workers are done;
        while(inQueue.get() != 0);
        tpe.shutdown();

        // Sort the Reduce result by its rank;
        Collections.sort(finalResult);

        // Print to file the result;
        for(int i = 0; i < finalResult.size(); i++) {
            try {
                Tema2.outputFile.write(finalResult.get(i).fileName.replace("tests/files/", "") + "," + 
                String.format("%.2f", finalResult.get(i).result) + "," + finalResult.get(i).maxSize 
                    + "," + finalResult.get(i).numberOfWordsWithMaxSize + "\n");
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        // close the input and output files;
        try {
            inputFile.close();
            outputFile.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
