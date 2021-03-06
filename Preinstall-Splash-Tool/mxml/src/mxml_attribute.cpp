/* 
   Mini XML lib PLUS for C++

   Attribute class

   Author: Giancarlo Niccolai <gian@niccolai.ws>

   $Id: mxml_attribute.cpp,v 1.2 2004/01/11 13:32:17 jonnymind Exp $
*/

#include <mxml.h>
#include <mxml_attribute.h>
#include <mxml_error.h>
#include <mxml_utility.h>

#include <ctype.h>

#include <cassert>

namespace MXML {


Attribute::Attribute( std::istream &in, int style, int l, int p ):
   Element( l, p )
{
   char chr, quotechr;
   int iStatus = 0;
   std::string entity;
   markBegin(); // default start


   m_value = "";
   m_name = "";

   while ( iStatus < 6 && ( in.get(chr) ) ) {
      //std::cout << "LINE: " << line() << "  POS: " << character() << std::endl;
      switch ( iStatus ) {
         // begin
         case 0:
            // no attributes found - should not happen as I have been called by
            // node only if an attribute is to be read.
            assert( chr != '>' && chr !='/');
            switch ( chr ) {
               case MXML_LINE_TERMINATOR: nextLine(); break;
               // We repeat line terminator here for portability
               case MXML_SOFT_LINE_TERMINATOR: break;
               case ' ': case '\t': 
                  throw new MalformedError( Error::errInvalidAtt, this );
                  
               default:
                  if ( isalpha( chr ) ) {
                     m_name = chr;
                     iStatus = 1;
                     markBegin();
                  }
                  else {
                     throw new MalformedError( Error::errInvalidAtt, this );
                  }
            }
         break;

         // scanning for a name
         case 1:
            if ( isalnum( chr ) || chr == '_' || chr == '-' || chr == ':' ) {
               m_name += chr;
            }
            else if( chr == MXML_LINE_TERMINATOR ) {
               nextLine();
               iStatus = 2; // waiting for a '='
            }
            // We repeat line terminator here for portability
            else if ( chr == ' ' || chr == '\t' || chr == '\n' || chr == '\r' ) {
               iStatus = 2;
            }
            else if ( chr == '=' ) {
               iStatus = 3;
            }
            else {
               throw MalformedError( Error::errMalformedAtt, this );
            }
         break;

         // waiting for '='
         case 2:
            if ( chr == '=' ) {
               iStatus = 3;
            }
            else if( chr == MXML_LINE_TERMINATOR ) {
               nextLine() ;
            }
            // We repeat line terminator here for portability
            else if ( chr == ' ' || chr == '\t' || chr == '\n' || chr == '\r' ) {
            }
            else {
               throw MalformedError( Error::errMalformedAtt, this );
            }
         break;

         // waiting for ' or "
         case 3:
            if ( chr == '\'' || chr == '"' ) {
               iStatus = 4;
               quotechr = chr;
            }
            else if( chr == MXML_LINE_TERMINATOR ) {
               nextLine() ;
            }
            // We repeat line terminator here for portability
            else if ( chr == ' ' || chr == '\t' || chr == '\n' || chr == '\r' ) {
            }
            else {
               throw MalformedError( Error::errMalformedAtt, this );
            }
         break;

         // scanning the attribute content ( until next quotechr )
         case 4:
            if ( chr == quotechr ) {
               iStatus = 6;
            }
            else if ( chr == '&' && !( style & MXML_STYLE_NOESCAPE) ) {
               iStatus = 5;
               entity = "";
            }
            else if( chr == MXML_LINE_TERMINATOR ) {
               nextLine();
               m_value += chr;
            }
            else {
               m_value += chr;
            }
         break;

         case 5:
            if ( chr == quotechr ) {
               iStatus = 6;
            }
            else if ( chr == ';' ) {
               if ( entity == "" ) {
                  throw MalformedError( Error::errWrongEntity, this );
               }

               iStatus = 4;

               // we see if we have a predef entity (also known as escape)
               if ( entity == "amp" ) chr = '&';
               else if ( entity == "lt" ) chr = '<';
               else if ( entity == "gt" ) chr = '>';
               else if ( entity == "quot" ) chr = '"';
               else if ( entity == "apos" ) chr = '\'';
               else {
                  // for now we take save the unexisting entity
                  chr = ';';
                  m_value += "&" + entity;
               }

               m_value += chr;
            }
            else if ( !isalpha( chr ) ) {
               //error
               throw MalformedError( Error::errUnclosedEntity, this );
            }
            else {
               entity += chr;
            }
         break;
      }
      nextChar();

   }

   if ( in.fail() ) throw IOError( Error::errIo, this );

   if ( iStatus < 6 ) {
      throw MalformedError( Error::errMalformedAtt, this );
   }
}

void Attribute::write( std::ostream &out, const int style ) const
{
   out << m_name << "=\"";

   if ( style & MXML_STYLE_NOESCAPE )
      out << m_value;
   else
      MXML::writeEscape( out, m_value );

   out << "\"";
}

}

/* end of mxml_attribute.cpp */
