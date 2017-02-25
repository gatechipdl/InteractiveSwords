import java.awt.datatransfer.*;
import java.awt.Toolkit;
import javax.swing.JOptionPane;
import ddf.minim.*;
//import minim.*;

int SAMPLES = 30000;

Minim minim;
AudioSample sample;

void setup()
{
  size(512, 200);
  
  String file = "";
  file = "C:/Sync/Projects/Work/InteractiveSwords/samples/clang/27858_erdie_sword04_converted.wav";
  file = "C:/Sync/Projects/Work/InteractiveSwords/samples/clang/52458_audione_sword-01_converted.wav";
  file = "C:/Sync/Projects/Work/InteractiveSwords/samples/clang/175949_freefire66_my-sword-hit-1-wav_converted.wav";
  file = "C:/Sync/Projects/Work/InteractiveSwords/samples/clang/275159_bird-man_sword-clash_converted.wav";
  file = "C:/Sync/Projects/Work/InteractiveSwords/samples/clang/170598_alkanetexe_anime-sword-sfx_converted.wav";
  
  
  /*String file = selectInput("Select audio file to encode.");
  File file = new File("");
  selectInput("Select audio file to encode.","fileSelected",file);

  if (file == null) {
    exit();
    return;
  }*/
  
  try {
    Clipboard clipboard = Toolkit.getDefaultToolkit().getSystemClipboard();
  
    minim = new Minim(this);
    sample = minim.loadSample(file);
    
    float[] samples = sample.getChannel(AudioSample.LEFT);
    float maxval = 0;
  
    for (int i = 0; i < samples.length; i++) {
      if (abs(samples[i]) > maxval) maxval = samples[i];
    }
    
    int start;
    
    for (start = 0; start < samples.length; start++) {
      if (abs(samples[start]) / maxval > 0.01) break;
    }
  
    String result = "";  
    for (int i = start; i < samples.length && i - start < SAMPLES; i++) {
      result += constrain(int(map(samples[i], -maxval, maxval, 0, 256)), 0, 255) + ", ";
    }
  
    clipboard.setContents(new StringSelection(result), null);
    
    JOptionPane.showMessageDialog(null, "Audio data copied to the clipboard.", "Success!", JOptionPane.INFORMATION_MESSAGE);
  } catch (Exception e) {
    JOptionPane.showMessageDialog(null, "Maybe you didn't pick a valid audio file?\n" + e, "Error!", JOptionPane.ERROR_MESSAGE);
  }
  
  exit();
}

void stop()
{
  sample.close();
  minim.stop();
  super.stop();
}

void fileSelected(File selection) {
  if (selection == null) {
    println("Window was closed or the user hit cancel.");
  } else {
    println("User selected " + selection.getAbsolutePath());
  }
}