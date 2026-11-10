#include "firfilter.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <cmath>

firfilter::firfilter(QObject *parent) : QObject(parent), prc_com(false) {

  this->clear();
}

firfilter::firfilter(const firfilter &fil, QObject *parent) : QObject(parent), prc_com(false) {
  this->clear();
  this->operator=(fil);
}

firfilter::~firfilter() {
}

void firfilter::copy_data(firfilter *fil) {
  this->clear();
  prc_com::operator=(*fil);
}

firfilter &firfilter::operator=(const firfilter &fil) {
  if (&fil == this)
    return *this;
  this->clear();
  prc_com::operator=(fil);
  return *this;
}

void firfilter::clear() {
  this->coeff.clear();
  this->QMap<QString, QVariant>::clear();
  insert("filter_name", QString("empty"));
  insert("filter_type", QString("empty"));
  insert("filter_long_description", QString("empty"));
  insert("filter_category", QString("empty"));
  insert("filter_move", (quint64)0);
  insert("filter_length", (quint64)0);
  insert("filter_outputs", (quint64)0);
  insert("sample_freq", (double)0.);
  insert("num_samples", (quint64)0);

  insert("upper_freq", (double)0.0);
  insert("lower_freq", (double)0.0);

  strs_opts << "mtx64" << "mtx64_3s" << "mtx32" << "mtx16" << "mtx16_2s" << "mtx8" << "mtx4" << "mtx2" << "mtx25" << "mtx10" << "mtx15" << "notch" << "highpass" << "lowpass" << "bandpass";
  if (this->coeff.size())
    this->coeff.clear();
}

void firfilter::clear_coefficients() {
  this->coeff.clear();
}

double firfilter::fir_fold(const std::vector<double> &tsdata_in) {
  double sum = 0;
  std::vector<double>::const_iterator coef_first = this->coeff.begin();
  std::vector<double>::const_iterator coef_last = this->coeff.end();
  std::vector<double>::const_iterator first = tsdata_in.begin();

  while (coef_first != coef_last) {
    sum += ((*first) * (*coef_first));
    ++coef_first;
    ++first;
  }
  return sum;
}

bool firfilter::set_firfilter(const prc_com &filter_part) {
  this->set_data(filter_part.get_data(), false);
  //    qDebug() << map_strs_opts.value("filter_name");
  //    qDebug() << svalue("filter_name");
  filter_part.show_my_map();
  if (!strs_opts.contains(svalue("filter_name"))) {
    qDebug() << "firfilter::set_filter" << "filter does not exist";
    this->clear();
    return false;
  }
  return true;
}

bool firfilter::load_coefficients(const QString &filter_name) {
  this->coeff.clear();
  const QFileInfo db_file = fall_back_default_str("filter.db");
  if (!db_file.isFile()) {
    qWarning() << "firfilter::load_coefficients database not found:" << db_file.absoluteFilePath();
    return false;
  }

  const QString connection_name = "firfilter_" + QUuid::createUuid().toString();
  bool success = false;
  {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connection_name);
    db.setConnectOptions("QSQLITE_OPEN_READONLY");
    db.setDatabaseName(db_file.absoluteFilePath());
    if (!db.open()) {
      qWarning() << "firfilter::load_coefficients cannot open database:" << db.lastError().text();
    } else {
      QString table_name = filter_name;
      if (filter_name == "mtx16_2s")
        table_name = "mtx16";
      else if (filter_name == "mtx64_3s")
        table_name = "mtx64";
      QSqlQuery query(db);
      if (!query.exec("SELECT coeff FROM \"" + table_name + "\" ORDER BY rowid")) {
        qWarning() << "firfilter::load_coefficients query failed:" << table_name << query.lastError().text();
      } else {
        this->coeff.reserve(this->sztvalue("filter_length"));
        bool valid = true;
        while (query.next()) {
          bool ok = false;
          const double value = query.value(0).toDouble(&ok);
          if (!ok || !std::isfinite(value)) {
            qWarning() << "firfilter::load_coefficients invalid coefficient in" << table_name;
            valid = false;
            break;
          }
          this->coeff.push_back(value);
        }
        if (query.lastError().isValid()) {
          qWarning() << "firfilter::load_coefficients read failed:" << table_name << query.lastError().text();
          valid = false;
        }
        if (this->coeff.size() != this->sztvalue("filter_length")) {
          qWarning() << "firfilter::load_coefficients unexpected coefficient count in" << table_name
                     << this->coeff.size() << "expected" << this->sztvalue("filter_length");
          valid = false;
        }
        success = valid;
      }
    }
  }
  QSqlDatabase::removeDatabase(connection_name);
  if (!success)
    this->coeff.clear();
  return success;
}

std::vector<double> firfilter::generate_coefficients(const QString &filter_name, bool &success, const bool create_coeff_vector,
                                                     const double sample_freq, const quint64 num_samples) {
  this->set_key_value("num_samples", num_samples);

  if (0 == filter_name.compare("mtx64") || 0 == filter_name.compare("mtx64_3s")) {

    insert("filter_name", filter_name);
    insert("filter_type", QString("decimation"));
    insert("filter_long_description", QString("metronix 64x decimation FIR with Remez"));
    insert("filter_category", QString("FIR"));
    insert("filter_move", (quint64)64);
    insert("filter_length", (quint64)1249);
    insert("filter_outputs", (quint64)1);
    if ((this->quint64value("filter_length") > this->quint64value("num_samples")) && this->quint64value("num_samples")) {
      success = false;
      if (create_coeff_vector)
        return this->coeff;

    } else
      success = true;
    if (create_coeff_vector) {
      success = this->load_coefficients(filter_name);
    }
  }

  if (0 == filter_name.compare("mtx32")) {

    insert("filter_name", filter_name);
    insert("filter_type", QString("decimation"));
    insert("filter_long_description", QString("metronix 32x decimation FIR with Remez"));
    insert("filter_category", QString("FIR"));
    insert("filter_move", (quint64)32);

    insert("filter_length", (quint64)471);
    insert("filter_outputs", (quint64)1);

    if ((this->quint64value("filter_length") > this->quint64value("num_samples")) && this->quint64value("num_samples")) {
      success = false;
      if (create_coeff_vector)
        return this->coeff;

    } else
      success = true;

    if (create_coeff_vector) {
      success = this->load_coefficients(filter_name);
    }
  }

  if (0 == filter_name.compare("mtx16") || 0 == filter_name.compare("mtx16_2s")) {

    insert("filter_name", filter_name);
    insert("filter_type", QString("decimation"));
    insert("filter_long_description", QString("metronix 16x decimation FIR with Remez"));
    insert("filter_category", QString("FIR"));
    insert("filter_move", (quint64)16);
    insert("filter_length", (quint64)313);
    insert("filter_outputs", (quint64)1);
    if ((this->quint64value("filter_length") > this->quint64value("num_samples")) && this->quint64value("num_samples")) {
      success = false;
      if (create_coeff_vector)
        return this->coeff;

    } else
      success = true;
    if (create_coeff_vector) {
      success = this->load_coefficients(filter_name);
    }
  }

  if (0 == filter_name.compare("mtx4")) {

    insert("filter_name", filter_name);
    insert("filter_type", QString("decimation"));
    insert("filter_long_description", QString("metronix 4x decimation FIR with Remez"));
    insert("filter_category", QString("FIR"));
    insert("filter_move", (quint64)4);

    insert("filter_length", (quint64)71);
    insert("filter_outputs", (quint64)1);

    if ((this->quint64value("filter_length") > this->quint64value("num_samples")) && this->quint64value("num_samples")) {
      success = false;
      if (create_coeff_vector)
        return this->coeff;

    } else
      success = true;

    if (create_coeff_vector) {
      success = this->load_coefficients(filter_name);
    }
  }

  if (0 == filter_name.compare("mtx25")) {

    insert("filter_name", filter_name);
    insert("filter_type", QString("decimation"));
    insert("filter_long_description", QString("metronix 25x decimation FIR with Remez"));
    insert("filter_category", QString("FIR"));
    insert("filter_move", (quint64)25);

    insert("filter_length", (quint64)501);
    insert("filter_outputs", (quint64)1);

    if ((this->quint64value("filter_length") > this->quint64value("num_samples")) && this->quint64value("num_samples")) {
      success = false;
      if (create_coeff_vector)
        return this->coeff;

    } else
      success = true;

    if (create_coeff_vector) {
      success = this->load_coefficients(filter_name);
    }
  }

  if (0 == filter_name.compare("mtx10")) {
    insert("filter_name", filter_name);
    insert("filter_type", QString("decimation"));
    insert("filter_long_description", QString("metronix 10x decimation FIR with Remez"));
    insert("filter_category", QString("FIR"));
    insert("filter_move", (quint64)10);

    insert("filter_length", (quint64)393);
    insert("filter_outputs", (quint64)1);

    if ((this->quint64value("filter_length") > this->quint64value("num_samples")) && this->quint64value("num_samples")) {
      success = false;
      if (create_coeff_vector)
        return this->coeff;

    } else
      success = true;

    if (create_coeff_vector) {
      success = this->load_coefficients(filter_name);
    }
  }

  if (0 == filter_name.compare("mtx2")) {
    insert("filter_name", filter_name);
    insert("filter_type", QString("decimation"));
    insert("filter_long_description", QString("metronix 2x decimation FIR with Remez"));
    insert("filter_category", QString("FIR"));
    insert("filter_move", (quint64)2);

    insert("filter_length", (quint64)35);
    insert("filter_outputs", (quint64)1);
    if ((this->quint64value("filter_length") > this->quint64value("num_samples")) && this->quint64value("num_samples")) {
      success = false;
      if (create_coeff_vector)
        return this->coeff;

    } else
      success = true;

    if (create_coeff_vector) {
      success = this->load_coefficients(filter_name);
    }
  }
  if (0 == filter_name.compare("mtx15")) {
    insert("filter_name", filter_name);
    insert("filter_type", QString("decimation"));
    insert("filter_long_description", QString("metronix 15x decimation FIR with Remez"));
    insert("filter_category", QString("FIR"));
    insert("filter_move", (quint64)15);

    insert("filter_length", (quint64)193);
    insert("filter_outputs", (quint64)1);
    if ((this->quint64value("filter_length") > this->quint64value("num_samples")) && this->quint64value("num_samples")) {
      success = false;
      if (create_coeff_vector)
        return this->coeff;

    } else
      success = true;

    if (create_coeff_vector) {
      success = this->load_coefficients(filter_name);
    }
  }

  if (0 == filter_name.compare("mtx8")) {
    insert("filter_name", filter_name);
    insert("filter_type", QString("decimation"));
    insert("filter_long_description", QString("metronix 4x decimation FIR with Remez"));
    insert("filter_category", QString("FIR"));
    insert("filter_move", (quint64)8);
    coeff.reserve(159);
    coeff.resize(159);
    insert("filter_length", (quint64)159);
    insert("filter_outputs", (quint64)1);

    if ((this->quint64value("filter_length") > this->quint64value("num_samples")) && this->quint64value("num_samples")) {
      success = false;
      if (create_coeff_vector)
        return this->coeff;

    } else
      success = true;

    if (create_coeff_vector) {
      success = this->load_coefficients(filter_name);
    }
  }
  if (0 == filter_name.compare("notch")) {

    insert("filter_name", filter_name);
    insert("filter_type", QString("notch"));
    insert("filter_long_description", QString("FIR notch filter"));
    insert("filter_category", QString("FIR"));
    insert("filter_move", (quint64)1);

    this->set_key_value("sample_freq", sample_freq);
    this->set_key_value("num_samples", num_samples);

    if (sample_freq > 0.99) {
      double tmp = (sample_freq * 2. + 1.);
      ;
      if (tmp < 513) {
        insert("filter_length", (quint64)513);
      } else {
        quint64 l = (quint64)tmp;
        // even needs to be odd
        if ((l % 2) == 0)
          ++l;
        insert("filter_length", l);
      }
    }

    if ((this->quint64value("filter_length") > this->quint64value("num_samples")) && this->quint64value("num_samples")) {
      success = false;
      if (create_coeff_vector)
        return this->coeff;

    } else
      success = true;

    insert("filter_outputs", (quint64)1);

    // the notch swaps the values to indicate notch !!!!! !!!!!!!!!!!!!!!!!!!!!!!!!
    double CutLow = this->dvalue("upper_freq");
    double CutHigh = this->dvalue("lower_freq");

    if (create_coeff_vector) {
      this->coeff.resize(this->sztvalue("filter_length"));
      iter::fir_dsgn<double, std::vector<double>::iterator>(coeff.begin(), coeff.end(), this->dvalue("sample_freq"), CutLow, CutHigh);
    }
  }

  if (0 == filter_name.compare("highpass")) {

    insert("filter_name", filter_name);
    insert("filter_type", QString("highpass"));
    insert("filter_long_description", QString("FIR highpass filter"));
    insert("filter_category", QString("FIR"));
    insert("filter_move", (quint64)1);

    this->set_key_value("sample_freq", sample_freq);
    this->set_key_value("num_samples", num_samples);

    insert("filter_outputs", (quint64)1);

    if (sample_freq > 0.999) {
      quint64 l = (quint64)sample_freq * 4;
      if (l < 8193)
        insert("filter_length", 8193);
      else {
        if ((l % 2) == 0)
          ++l;
        insert("filter_length", l);
      }
    } else {
      insert("filter_length", 1025);
    }

    if ((this->quint64value("filter_length") > this->quint64value("num_samples")) && this->quint64value("num_samples")) {
      success = false;
      if (create_coeff_vector)
        return this->coeff;

    } else
      success = true;

    if (create_coeff_vector) {
      this->coeff.resize(this->sztvalue("filter_length"));
      iter::fir_dsgn<double, std::vector<double>::iterator>(coeff.begin(), coeff.end(), this->dvalue("sample_freq"), this->dvalue("lower_freq"), 1.0);
    }
  }

  if (0 == filter_name.compare("bandpass")) {

    insert("filter_name", filter_name);
    insert("filter_type", QString("bandpass"));
    insert("filter_long_description", QString("FIR bandpass filter"));
    insert("filter_category", QString("FIR"));
    insert("filter_move", (quint64)1);

    this->set_key_value("sample_freq", sample_freq);
    this->set_key_value("num_samples", num_samples);

    insert("filter_outputs", (quint64)1);

    if (sample_freq > 0.999) {
      quint64 l = (quint64)sample_freq * 4;
      if (l < 8193)
        insert("filter_length", 8193);
      else {
        if ((l % 2) == 0)
          ++l;
        insert("filter_length", l);
      }
    } else {
      insert("filter_length", 1025);
    }

    if ((this->quint64value("filter_length") > this->quint64value("num_samples")) && this->quint64value("num_samples")) {
      success = false;
      if (create_coeff_vector)
        return this->coeff;

    } else
      success = true;

    if (create_coeff_vector) {
      this->coeff.resize(this->sztvalue("filter_length"));
      iter::fir_dsgn<double, std::vector<double>::iterator>(coeff.begin(), coeff.end(), this->dvalue("sample_freq"), this->dvalue("lower_freq"), dvalue("upper_freq"));
    }
  }

  if (0 == filter_name.compare("lowpass")) {

    insert("filter_name", filter_name);
    insert("filter_type", QString("lowpass"));
    insert("filter_long_description", QString("FIR lowpass filter"));
    insert("filter_category", QString("FIR"));
    insert("filter_move", (quint64)1);
    insert("lower_freq", (double)0);

    this->set_key_value("sample_freq", sample_freq);
    this->set_key_value("num_samples", num_samples);

    // for above 128Hz this is always true
    double tmp = (sample_freq * 2. + 1.);
    ;
    if (tmp < 513) {
      insert("filter_length", (quint64)513);
    }
    // let the minimum of 257
    else {
      quint64 l = (quint64)tmp;
      // even needs to be odd
      if ((l % 2) == 0)
        ++l;
      insert("filter_length", l);
    }

    if ((this->quint64value("filter_length") > this->quint64value("num_samples")) && this->quint64value("num_samples")) {
      success = false;
      if (create_coeff_vector)
        return this->coeff;

    } else
      success = true;

    insert("filter_outputs", (quint64)1);

    if (create_coeff_vector) {
      this->coeff.resize(this->sztvalue("filter_length"));
      iter::fir_dsgn<double, std::vector<double>::iterator>(coeff.begin(), coeff.end(), this->dvalue("sample_freq"), 0.0, this->dvalue("upper_freq"));
    }
  }

  return this->coeff;
}
