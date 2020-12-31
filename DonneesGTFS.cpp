//
// Created by Mario Marchand on 16-12-29.
//
#include <stdio.h>
#include <string.h>
#include "DonneesGTFS.h"

using namespace std;

//! \brief ajoute les lignes dans l'objet GTFS
//! \param[in] p_nomFichier: le nom du fichier contenant les lignes
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterLignes(const std::string &p_nomFichier)
{
    //écrire votre code ici
    ifstream  fichierComplet(p_nomFichier);

    if (fichierComplet.fail()) {
        throw logic_error("Impossible de lire le fichier");
    }

    int index = 0;
    for (string ligne; getline(fichierComplet, ligne);)
    {
        if (index != 0) {
            ligne.erase(remove(ligne.begin(), ligne.end(), '"'), ligne.end());

            vector<string> vecteur_de_mots = string_to_vector(ligne, ',');

            int route_id = stoi(vecteur_de_mots.at(0));
            string p_numero = vecteur_de_mots.at(2);
            string p_description = vecteur_de_mots.at(4);
            string p_couleur = vecteur_de_mots.at(7);
            CategorieBus categorieBus = Ligne().couleurToCategorie(p_couleur);

            Ligne ligne(route_id, p_numero, p_description, categorieBus);

            m_lignes[ligne.getId()] = ligne;
            m_lignes_par_numero.insert({ ligne.getNumero(), ligne});
        }

        index++;

    }
}

//! \brief ajoute les stations dans l'objet GTFS
//! \param[in] p_nomFichier: le nom du fichier contenant les station
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterStations(const std::string &p_nomFichier)
{
    //écrire votre code ici
    ifstream  fichierComplet(p_nomFichier);

    if (fichierComplet.fail()) {
        throw logic_error("Impossible de lire le fichier");
    }

    int index = 0;
    for (string ligne; getline(fichierComplet, ligne);)
    {
        if (index != 0) {
            ligne.erase(remove(ligne.begin(), ligne.end(), '"'), ligne.end());

            vector<string> vecteur_de_mots = string_to_vector(ligne, ',');

            int stop_id = stoi(vecteur_de_mots.at(0));
            string stop_name = vecteur_de_mots.at(1);
            string stop_description = vecteur_de_mots.at(2);
            string stop_lattitude = vecteur_de_mots.at(3);
            string stop_longitude = vecteur_de_mots.at(4);

            Coordonnees coordonnees_station(stod(stop_lattitude), stod(stop_longitude));
            Station station(stop_id, stop_name, stop_description, coordonnees_station);

            m_stations[station.getId()] = station;
        }

        index++;

    }
}

//! \brief ajoute les transferts dans l'objet GTFS
//! \breif Cette méthode doit âtre utilisée uniquement après que tous les arrêts ont été ajoutés
//! \brief les transferts (entre stations) ajoutés sont uniquement ceux pour lesquelles les stations sont prensentes dans l'objet GTFS
//! \brief les transferts sont ajoutés dans m_transferts
//! \brief les from_station_id des stations de transfert sont ajoutés dans m_stationsDeTransfert
//! \param[in] p_nomFichier: le nom du fichier contenant les station
//! \throws logic_error si un problème survient avec la lecture du fichier
//! \throws logic_error si tous les arrets de la date et de l'intervalle n'ont pas été ajoutés
void DonneesGTFS::ajouterTransferts(const std::string &p_nomFichier)
{

    //écrire votre code ici
    ifstream fichierComplet(p_nomFichier);

    if (fichierComplet.fail()) {
        throw logic_error("Impossible de lire le fichier");
    }

    if (!m_tousLesArretsPresents) {
        throw logic_error("Tous les arrets de la date et de l'intervalle n'ont pas été ajoutés");

    }

    int index = 0;
    for (string ligne; getline(fichierComplet, ligne);)
    {
        if (index != 0) {
            ligne.erase(remove(ligne.begin(), ligne.end(), '"'), ligne.end());

            vector<string> vecteur_de_mots = string_to_vector(ligne, ',');

            unsigned int from_station_id = stoi(vecteur_de_mots.at(0));
            unsigned int to_station_id = stoi(vecteur_de_mots.at(1));
            unsigned int min_transfert_time = stoi(vecteur_de_mots.at(3));

            if (m_stations.count(from_station_id) && m_stations.count(to_station_id)) {
                if (min_transfert_time == 0) {
                    min_transfert_time = 1;
                }

                m_transferts.emplace_back(from_station_id, to_station_id, min_transfert_time);
                m_stationsDeTransfert.insert(from_station_id);
            }
        }

        index++;

    }

}


//! \brief ajoute les services de la date du GTFS (m_date)
//! \param[in] p_nomFichier: le nom du fichier contenant les services
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterServices(const std::string &p_nomFichier)
{

    //écrire votre code ici
    ifstream fichierComplet(p_nomFichier);

    if(fichierComplet.fail()) {
        throw new logic_error("Impossible d'ouvrir le fichier");
    }

    int index = 0;
    for (string ligne; getline(fichierComplet, ligne);)
    {
        if (index != 0) {
            vector<string> vecteur_de_mots = string_to_vector(ligne, ',');

            string service_id = vecteur_de_mots.at(0);
            string date_in_string = vecteur_de_mots.at(1);
            string exception_type = vecteur_de_mots.at(2);
            unsigned int annee_de_date = stoi(date_in_string.substr(0, 4));
            unsigned int mois_de_date = stoi(date_in_string.substr(4, 2));
            unsigned int jour_de_date = stoi(date_in_string.substr(6, 2));

            Date date_du_service(annee_de_date, mois_de_date, jour_de_date);

            if(exception_type == "1" && m_date == date_du_service) {
                m_services.insert(service_id);
            }
        }
        index++;

    }

}

//! \brief ajoute les voyages de la date
//! \brief seuls les voyages dont le service est présent dans l'objet GTFS sont ajoutés
//! \param[in] p_nomFichier: le nom du fichier contenant les voyages
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterVoyagesDeLaDate(const std::string &p_nomFichier)
{

    //écrire votre code ici
    ifstream fichierComplet(p_nomFichier);

    if(fichierComplet.fail()) {
        throw new logic_error("Impossible d'ouvrir le fichier");
    }

    int index = 0;
    for (string ligne; getline(fichierComplet, ligne);)
    {
        if (index != 0) {
            ligne.erase(remove(ligne.begin(), ligne.end(), '"'), ligne.end());

            vector<string> vecteur_de_mots = string_to_vector(ligne, ',');

            if (m_services.count(vecteur_de_mots.at(1))) {
                Voyage voyage(vecteur_de_mots.at(2), stoi(vecteur_de_mots.at(0)), vecteur_de_mots.at(1),
                              vecteur_de_mots.at(3));
                m_voyages[voyage.getId()] = voyage;
            }
            }
        index++;
    }
}

//! \brief ajoute les arrets aux voyages présents dans le GTFS si l'heure du voyage appartient à l'intervalle de temps du GTFS
//! \brief Un arrêt est ajouté SSI son heure de départ est >= now1 et que son heure d'arrivée est < now2
//! \brief De plus, on enlève les voyages qui n'ont pas d'arrêts dans l'intervalle de temps du GTFS
//! \brief De plus, on enlève les stations qui n'ont pas d'arrets dans l'intervalle de temps du GTFS
//! \param[in] p_nomFichier: le nom du fichier contenant les arrets
//! \post assigne m_tousLesArretsPresents à true
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterArretsDesVoyagesDeLaDate(const std::string &p_nomFichier)
{

    //écrire votre code ici
    ifstream fichierComplet(p_nomFichier);

    if(fichierComplet.fail()) {
        throw new logic_error("Impossible d'ouvrir le fichier");
    }

    map<string, Voyage> voyage_contenant_arret;
    map<unsigned int, Station> station_contenant_arret;
    int index = 0;
    for (string ligne; getline(fichierComplet, ligne);)
    {
        if (index != 0) {
            ligne.erase(remove(ligne.begin(), ligne.end(), '"'), ligne.end());

            vector<string> vecteur_de_mots = string_to_vector(ligne, ',');

            string trip_id = vecteur_de_mots.at(0);
            string arrival_time = vecteur_de_mots.at(1);
            string departure_time = vecteur_de_mots.at(2);
            string station_id = vecteur_de_mots.at(3);
            string numero_sequence = vecteur_de_mots.at(4);

            Heure heure_arrivee(stoi(arrival_time.substr(0, 2)), stoi(arrival_time.substr(3, 2)), stoi(arrival_time.substr(6, 2)));
            Heure heure_de_depart(stoi(departure_time.substr(0, 2)), stoi(departure_time.substr(3, 2)), stoi(departure_time.substr(6, 2)));

            if (m_voyages.count(trip_id) && heure_arrivee.operator<(m_now2) && heure_de_depart.operator>=(m_now1)) {
                Arret::Ptr arret = make_shared<Arret>(stoi(station_id), heure_arrivee, heure_de_depart, stoi(numero_sequence), trip_id);

                m_voyages[trip_id].ajouterArret(arret);
                m_stations[stoi(station_id)].addArret(arret);
                m_nbArrets++;
            }

        }
        index++;
    }

    map<std::string, Voyage> copie_des_voyages(m_voyages);
    for (auto voyage : copie_des_voyages) {
        if (voyage.second.getNbArrets() == 0) {
            m_voyages.erase(voyage.first);
        }
    }

    map<unsigned int, Station> copie_des_stations(m_stations);
    for (auto station :copie_des_stations) {
        if (station.second.getNbArrets() == 0) {
            m_stations.erase(station.first);
        }
    }

    m_tousLesArretsPresents = true;
}



