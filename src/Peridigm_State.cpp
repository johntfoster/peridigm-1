/*! \file Peridigm_State.cpp */

//@HEADER
// ************************************************************************
//
//                             Peridigm
//                 Copyright (2011) Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions?
// David J. Littlewood   djlittl@sandia.gov
// John A. Mitchell      jamitch@sandia.gov
// Michael L. Parks      mlparks@sandia.gov
// Stewart A. Silling    sasilli@sandia.gov
//
// ************************************************************************
//@HEADER

#include "Peridigm_State.hpp"
#include <Epetra_Import.h>

void PeridigmNS::State::allocateScalarData(Teuchos::RCP< std::vector<Field_NS::FieldSpec> > fieldSpecs, Teuchos::RCP<const Epetra_BlockMap> map)
{
  std::vector<Field_NS::FieldSpec> sortedFieldSpecs(*fieldSpecs);
  std::sort(sortedFieldSpecs.begin(), sortedFieldSpecs.end());
  scalarData = Teuchos::rcp(new Epetra_MultiVector(*map, sortedFieldSpecs.size()));
  for(unsigned int i=0 ; i<sortedFieldSpecs.size() ; ++i)
    fieldSpecToDataMap[sortedFieldSpecs[i]] = Teuchos::rcp((*scalarData)(i), false);
}

void PeridigmNS::State::allocateVectorData(Teuchos::RCP< std::vector<Field_NS::FieldSpec> > fieldSpecs, Teuchos::RCP<const Epetra_BlockMap> map)
{
  std::vector<Field_NS::FieldSpec> sortedFieldSpecs(*fieldSpecs);
  std::sort(sortedFieldSpecs.begin(), sortedFieldSpecs.end());
  vectorData = Teuchos::rcp(new Epetra_MultiVector(*map, sortedFieldSpecs.size()));
  for(unsigned int i=0 ; i<sortedFieldSpecs.size() ; ++i)
    fieldSpecToDataMap[sortedFieldSpecs[i]] = Teuchos::rcp((*vectorData)(i), false);
}

void PeridigmNS::State::allocateBondData(Teuchos::RCP< std::vector<Field_NS::FieldSpec> > fieldSpecs, Teuchos::RCP<const Epetra_BlockMap> map)
{
  std::vector<Field_NS::FieldSpec> sortedFieldSpecs(*fieldSpecs);
  std::sort(sortedFieldSpecs.begin(), sortedFieldSpecs.end());
  bondData = Teuchos::rcp(new Epetra_MultiVector(*map, sortedFieldSpecs.size()));
  for(unsigned int i=0 ; i<sortedFieldSpecs.size() ; ++i)
    fieldSpecToDataMap[sortedFieldSpecs[i]] = Teuchos::rcp((*bondData)(i), false);
}

Teuchos::RCP< std::vector<Field_NS::FieldSpec> > PeridigmNS::State::getFieldSpecs(Teuchos::RCP<Field_NS::FieldSpec::FieldLength> fieldLength)
{
  Teuchos::RCP< std::vector<Field_NS::FieldSpec> > fieldSpecs = Teuchos::rcp(new std::vector<Field_NS::FieldSpec>);
  std::map< Field_NS::FieldSpec, Teuchos::RCP<Epetra_Vector> >::const_iterator it;
  for(it = fieldSpecToDataMap.begin() ; it != fieldSpecToDataMap.end() ; ++it){
    if(fieldLength.is_null() || it->first.getLength() == *fieldLength)
      fieldSpecs->push_back(it->first);
  }
  return fieldSpecs;
}

Teuchos::RCP<Epetra_Vector> PeridigmNS::State::getData(Field_NS::FieldSpec fieldSpec)
{
  // search for the data
  std::map< Field_NS::FieldSpec, Teuchos::RCP<Epetra_Vector> >::iterator lb = fieldSpecToDataMap.lower_bound(fieldSpec);
  // if the key does not exist, throw an exception
  bool keyExists = ( lb != fieldSpecToDataMap.end() && !(fieldSpecToDataMap.key_comp()(fieldSpec, lb->first)) );
  TEST_FOR_EXCEPTION(!keyExists, Teuchos::RangeError, 
                       "Error in PeridigmNS::State::getData(), key does not exist!");
  return lb->second;
}

void PeridigmNS::State::copyFrom(PeridigmNS::State& source)
{
  Epetra_Import scalarImporter(scalarData->Map(), source.getScalarMultiVector()->Map());
  scalarData->Import(*source.getScalarMultiVector(), scalarImporter, Insert);
  Epetra_Import vectorImporter(vectorData->Map(), source.getVectorMultiVector()->Map());
  vectorData->Import(*source.getVectorMultiVector(), vectorImporter, Insert);
  Epetra_Import bondImporter(bondData->Map(), source.getBondMultiVector()->Map());
  bondData->Import(*source.getBondMultiVector(), bondImporter, Insert);
}
