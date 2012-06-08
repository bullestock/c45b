# qconf

include(../prefix.pri)

isEmpty ( PREFIX ) {
  PREFIX = /usr/local
}
isEmpty ( BINDIR ) {
  BINDIR = $${PREFIX}/bin
}
isEmpty ( DATADIR ) {
  DATADIR = $${PREFIX}/share
}
