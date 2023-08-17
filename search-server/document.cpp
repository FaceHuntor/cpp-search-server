#include "document.h"

using namespace std;

ostream& operator<<(ostream& out, const Document& d){
    out << "{ document_id = " << d.id << ", relevance = " << d.relevance << ", rating = " << d.rating << " }";
    return out;
}