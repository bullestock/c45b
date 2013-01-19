# Common install paths definitions. On Unix-like operating systems it
# is possible to tweak the install-prefix by running qmake like this:
#
# qmake PREFIX=MY_VERY_OWN_LOCATION                                                                                                                                                                 
#                                                                                                                                                                                                   
# e.g. "qmake PREFIX=$HOME/software/" or the like.                                                                                                                                                  
#                                                                                                                                                                                                   
                                                                                                                                                                                                    
win32 {                                                                                                                                                                                             
  EXEC_DIR = "$$(PROGRAMFILES_HOME)"                                                                                                                                                                
}                                                                                                                                                                                                   
                                                                                                                                                                                                    
unix {                                                                                                                                                                                              
  isEmpty( PREFIX ) {                                                                                                                                                                               
    PREFIX = /usr                                                                                                                                                                                   
  }                                                                                                                                                                                                 
  isEmpty( EXEC_DIR ) {                                                                                                                                                                             
    EXEC_DIR = $${PREFIX}/bin                                                                                                                                                                       
  }                                                                                                                                                                                                 
  isEmpty( LIB_DIR ) {                                                                                                                                                                              
    LIB_DIR = $${PREFIX}/lib                                                                                                                                                                        
  }                                                                                                                                                                                                 
  isEmpty( INCLUDE_DIR ) {                                                                                                                                                                          
    INCLUDE_DIR = $${PREFIX}/include                                                                                                                                                                
  }                                                                                                                                                                                                 
  isEmpty( DOC_DIR ) {                                                                                                                                                                              
    macx|darwin-g++ {                                                                                                                                                                               
      DOC_DIR = /Developer/Documentation/$${TARGET}                                                                                                                                                 
    } else {                                                                                                                                                                                        
      DOC_DIR = $${PREFIX}/share/doc/$${TARGET}                                                                                                                                                     
    }                                                                                                                                                                                               
  }                                                                                                                                                                                                 
}