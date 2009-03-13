#if !defined(STOR_PROGRESSMARKER_H)
#define STOR_PROGRESSMARKER_H

namespace stor 
{
  class ProgressMarker
    {
    public:
      static ProgressMarker *instance();
      static void instance(ProgressMarker *);    
      std::string status();

      void reading(bool b)    {reading_    = b;}
      void writing(bool b)    {writing_    = b;}
      void processing(bool b) {processing_ = b;}      
      
      std::string idle()    { return "Idle";}
      std::string input()   { return "Input";} 
      std::string process() { return "Process";}
      std::string output()  { return "Output"; }

    private:
      ProgressMarker();
      
      bool reading_;
      bool writing_;
      bool processing_;
    }; 
}

#endif

