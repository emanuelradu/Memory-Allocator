# Memory-Allocator

Pentru usurarea gestionarii blocurilor din arena am considerat utila
utilizarea unei structuri, block, ce contine indexul de inceput al blocului,
indexul zonei de date a blocului, numarul de octeti ai zonei de date, indexul
blocului anterior, indexul blocului urmator, adresa structurii block
corespunzatoare blocului anterior si adresa structurii block corespunzatoare
blocului urmator.

Ca variabile globale, pe langa pointerul arena si dimensiunea acesteia, am
2 pointeri de tip block, unul indicand catre primul block al arenei, celalalt
catre ultimul block al arenei si o variabila in care pastrez numarul de blocuri
din arena.

Maparea unei structuri de tip block in arena, se face folosind functia
map_block_to_arena, care scrie zona de gestiune a blocului in arena.
Demaparea unei structuri de tip block din arena, se face folosind functia
unmap_block_from_arena, care seteaza pe 0 toti octetii blocului si elibereaza
memoria folosita de structura block.

Alocarea unui bloc este impartita in 4 cazuri: cand nu mai exista nici un
bloc alocat, exista loc alocarii inaintea primului bloc, intre 2 blocuri,
dupa ultimul bloc.
	
Stergerea unui bloc este impartita de asemenea in 4 cazuri: cand blocul
curent nu are blocuri vecine, cand are bloc vecin doar in stanga, cand are bloc
vecin doar in dreapta, cand are bloc vecin si in stanga si in dreapta.

In functie de cazul de la alocare sau stergere se modifica corespunzator
pointerii ce indica prima si ultima structura block, numarul de blocuri,
pointerii next / prev ai blocului adaugat / sters, cat si ai blocurilor vecine.

La executarea comenzii FINALIZE, este eliberata memoria tuturor strctutilor
block folosite.
