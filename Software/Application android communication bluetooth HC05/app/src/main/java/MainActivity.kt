package com.example.chasourapp_v1

import android.Manifest
import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothSocket
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.widget.*
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import java.io.IOException
import java.io.InputStream
import java.io.OutputStream
import java.util.*
import kotlin.concurrent.thread

class MainActivity : AppCompatActivity() {
    private val SPP_UUID: UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")

    private lateinit var tvStatus: TextView
    private lateinit var spinner: Spinner
    private lateinit var btnConnect: Button
    private lateinit var btnScan: Button
    private lateinit var btnOn: Button
    private lateinit var btnOff: Button
    private lateinit var btnSend: Button
    private lateinit var etMessage: EditText
    private lateinit var tvLog: TextView

    private val bluetoothAdapter: BluetoothAdapter? = BluetoothAdapter.getDefaultAdapter()
    private var deviceList = mutableListOf<BluetoothDevice>()
    private var socket: BluetoothSocket? = null
    private var input: InputStream? = null
    private var output: OutputStream? = null
    private var readerThread: Thread? = null

    private val requestPermissionsLauncher = registerForActivityResult(
        ActivityResultContracts.RequestMultiplePermissions()
    ) { perms ->
        // pas d'action spécifique : on hypothèque que l'utilisateur accepte
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        tvStatus = findViewById(R.id.tvStatus)
        spinner = findViewById(R.id.spinnerDevices)
        btnConnect = findViewById(R.id.btnConnect)
        btnScan = findViewById(R.id.btnScan)
        btnOn = findViewById(R.id.btnOn)
        btnOff = findViewById(R.id.btnOff)
        btnSend = findViewById(R.id.btnSend)
        etMessage = findViewById(R.id.etMessage)
        tvLog = findViewById(R.id.tvLog)

        checkBluetoothAvailable()
        requestBluetoothPermissions()

        btnScan.setOnClickListener { scanPairedDevices() }
        btnConnect.setOnClickListener { connectToSelectedDevice() }
        btnOn.setOnClickListener { sendText("ON\r\n") }
        btnOff.setOnClickListener { sendText("OFF\r\n") }
        btnSend.setOnClickListener {
            val txt = etMessage.text.toString()
            if (txt.isNotEmpty()) {
                sendText(txt + "\r\n")
            }
        }

        scanPairedDevices()
    }

    private fun checkBluetoothAvailable() {
        if (bluetoothAdapter == null) {
            toast("Bluetooth non supporté")
            finish()
            return
        }
        if (!bluetoothAdapter.isEnabled) {
            // On pourrait demander activation via Intent, ici simplifié
            toast("Active le Bluetooth et appaire le HC-05 à l'avance (PIN 1234)")
        }
    }

    private fun requestBluetoothPermissions() {
        val perms = mutableListOf<String>()
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            perms.add(Manifest.permission.BLUETOOTH_SCAN)
            perms.add(Manifest.permission.BLUETOOTH_CONNECT)
            perms.add(Manifest.permission.BLUETOOTH_ADVERTISE)
        } else {
            perms.add(Manifest.permission.ACCESS_FINE_LOCATION)
        }
        requestPermissionsLauncher.launch(perms.toTypedArray())
    }

    @SuppressLint("MissingPermission")
    private fun scanPairedDevices() {
        deviceList.clear()
        val paired = bluetoothAdapter?.bondedDevices
        val names = mutableListOf<String>()
        paired?.forEach {
            deviceList.add(it)
            names.add("${it.name} — ${it.address}")
        }
        if (names.isEmpty()) names.add("Aucun appareil appairé")
        val adapter = ArrayAdapter(this, android.R.layout.simple_spinner_item, names)
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item)
        spinner.adapter = adapter
        tvStatus.text = "Appareils appairés listés"
    }

    @SuppressLint("MissingPermission")
    private fun connectToSelectedDevice() {
        val pos = spinner.selectedItemPosition
        if (pos < 0 || pos >= deviceList.size) {
            toast("Sélectionne un appareil HC-05 appairé")
            return
        }
        val device = deviceList[pos]
        tvStatus.text = "Connexion en cours à ${device.name}"
        thread {
            try {
                // close si existant
                socket?.close()
                // tentative de connection RFCOMM SPP
                socket = device.createRfcommSocketToServiceRecord(SPP_UUID)
                bluetoothAdapter?.cancelDiscovery()
                socket?.connect()
                input = socket?.inputStream
                output = socket?.outputStream
                runOnUiThread { tvStatus.text = "Connecté à ${device.name}" }
                startReader()
            } catch (e: IOException) {
                e.printStackTrace()
                runOnUiThread {
                    tvStatus.text = "Erreur connexion: ${e.message}"
                    toast("Erreur connexion: ${e.message}")
                }
                try { socket?.close() } catch (_: Exception) {}
            }
        }
    }

    private fun startReader() {
        readerThread?.interrupt()
        readerThread = thread {
            val buffer = ByteArray(1024)
            try {
                while (!Thread.currentThread().isInterrupted) {
                    val read = input?.read(buffer) ?: -1
                    if (read > 0) {
                        val s = String(buffer, 0, read)
                        runOnUiThread {
                            tvLog.append(s)
                            // autoscroll
                            val scrollAmount = tvLog.layout?.getLineTop(tvLog.lineCount) ?: 0
                            if (scrollAmount > tvLog.height) tvLog.scrollTo(0, scrollAmount - tvLog.height)
                        }
                    } else if (read < 0) {
                        // déconnecté
                        runOnUiThread { tvStatus.text = "Déconnecté" }
                        break
                    }
                }
            } catch (e: IOException) {
                runOnUiThread { tvStatus.text = "Lecture arrêtée: ${e.message}" }
            }
        }
    }

    private fun sendText(txt: String) {
        thread {
            try {
                output?.write(txt.toByteArray())
                runOnUiThread { tvLog.append("> $txt") }
            } catch (e: IOException) {
                runOnUiThread {
                    tvStatus.text = "Erreur envoi: ${e.message}"
                }
            }
        }
    }

    override fun onDestroy() {
        try {
            readerThread?.interrupt()
            socket?.close()
        } catch (_: Exception) {}
        super.onDestroy()
    }

    private fun toast(s: String) = runOnUiThread { Toast.makeText(this, s, Toast.LENGTH_SHORT).show() }
}