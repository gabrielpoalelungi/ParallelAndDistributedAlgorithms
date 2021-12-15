import java.io.BufferedReader;
import java.io.EOFException;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.StringTokenizer;

public class MapRunnable implements Runnable{
    RandomAccessFile currentFile;
    BufferedReader myBufferedReader;
    int currentFilePosition;
    int offset;
    int fragmentDimension;
    String fileName;

    MapRunnable(String fileName, int offset, int fragmentDimension) {
        this.fileName = fileName;
        this.offset = offset;
        this.fragmentDimension = fragmentDimension;
    }

    @Override
    public void run() {
        try {
            // Raw fragment to be from the current document;
            byte[] buff = new byte[fragmentDimension];
            currentFile = new RandomAccessFile("../" + fileName, "r");

            try {
                // Read the assigned fragment from the current document;
                currentFile.seek(offset);
                int nr = 0;
                while(nr < fragmentDimension) { 
                    buff[nr++] = currentFile.readByte();
                }

                // Make a list of words from the assigned fragment;
                StringTokenizer st = new StringTokenizer(new String(buff), ";:/?~\\.,><`[]{}()!@#$%^&-_+'=*\"| \t\r\n");
                List<String> aux_buff = new ArrayList<>();
                while(st.hasMoreTokens()) {
                    aux_buff.add(st.nextToken());
                }

                // If the fragment begins in the middle of a word, skip it;
                if(offset - 1 >= 0) {
                    currentFile.seek(offset - 1);
                    byte ch = currentFile.readByte();
                    if(((int)ch >= 65 && (int)ch <= 90) || ((int)ch >= 97 && (int)ch <= 122)) {
                        // and if the byte that succedes the offset is a letter
                        currentFile.seek(offset);
                        byte ch2 = currentFile.readByte();
                        if(((int)ch2 >= 65 && (int)ch2 <= 90) || ((int)ch2 >= 97 && (int)ch2 <= 122)) {
                            // remove the word from the list of words of this fragment;
                            aux_buff.remove(0);
                        }
                    }
                }

                // If the fragment ends in the middle of a word, read the whole word;
                long lengthOfFile = currentFile.length();
                if (offset + fragmentDimension < lengthOfFile) {
                    // If the byte that precedes the offset is a letter 
                    currentFile.seek(offset + fragmentDimension);
                    byte ch = currentFile.readByte();
                    if(((int)ch >= 65 && (int)ch <= 90) || ((int)ch >= 97 && (int)ch <= 122)) {
                        currentFile.seek(offset + fragmentDimension - 1);
                        byte ch2 = currentFile.readByte();
                        if(((int)ch2 >= 65 && (int)ch2 <= 90) || ((int)ch2 >= 97 && (int)ch2 <= 122)) {
                        // and if the byte that succedes the offset is a letter;
                            int OK = 1;
                            byte[] aux = new byte[fragmentDimension];
                            int cnt = 0;
                            // read bytes until it reads a non-letter byte;
                            while (OK == 1) {
                                try {
                                    byte new_ch = currentFile.readByte();
                                    if(((int)new_ch < 65 || (int)new_ch > 90) && ((int)new_ch < 97 || (int)new_ch > 122)){
                                        OK = 0;
                                    } else {
                                        aux[cnt] = new_ch;
                                        cnt++;
                                    }
                                } catch (EOFException e) {
                                    // stops if it reaches the end of file;
                                    break;
                                }
                            }
                            byte[] final_token = new byte[cnt];
                            for(int i = 0; i < cnt; i++) {
                                final_token[i] = aux[i];
                            }
                            // replace the last fragmented word with its whole version;
                            aux_buff.set(aux_buff.size() - 1, 
                                aux_buff.get(aux_buff.size() - 1).concat(new String(final_token)));
                        }
                    }
                }

                // Build the dictionary with keys = length of a word and value = number of words with key-length;
                HashMap<Integer, Integer> dictionary = new HashMap<>();
                // Build the list of words with maximum length;
                List<String> maxLengthWords = new ArrayList<>();
                int maxSize = 0;
                for(int i = 0; i < aux_buff.size(); i++) {
                    // If that length exists as key, increment number of appearances;
                    if(dictionary.containsKey(aux_buff.get(i).length())) {
                        dictionary.replace(aux_buff.get(i).length(), dictionary.get(aux_buff.get(i).length()) + 1); 
                    } else {
                        dictionary.put(aux_buff.get(i).length(), 1);
                    }
                    // if this word is the maximum size, add it in the list
                    if(aux_buff.get(i).length() == maxSize) {
                        maxLengthWords.add(aux_buff.get(i));
                    } 
                    // else if this word has greater size, clear the list and add this word in the list;
                    else if (aux_buff.get(i).length() > maxSize) {
                        maxSize = aux_buff.get(i).length();
                        maxLengthWords.clear();
                        maxLengthWords.add(aux_buff.get(i));
                    }
                }
                // Compute the result (fileName + the dictionary + list of maximum words) and add it in the list of Map
                // results in Tema2 main class;
                Tema2.resultsMap.add(new ResultMapRunnable(fileName, dictionary, maxLengthWords));

            } catch (IOException e) {
                e.printStackTrace();
            }
            // This worker finished this task;
            Tema2.inQueue.decrementAndGet();

        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
    }
}
