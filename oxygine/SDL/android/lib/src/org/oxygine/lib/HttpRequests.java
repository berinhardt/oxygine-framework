package org.oxygine.lib;

import android.annotation.TargetApi;
import android.content.Context;
import android.net.ConnectivityManager;
import android.os.Build;
import android.os.PowerManager;
import android.util.Log;

import java.io.*;
import java.net.HttpURLConnection;
import java.net.InetSocketAddress;
import java.net.Proxy;
import java.net.URL;
import java.util.concurrent.RejectedExecutionException;

import com.android.volley.*;
import com.android.volley.toolbox.*;
import java.util.Map;
import java.util.HashMap;
/**
 * Created by Denis on 31.12.2014.
 */

class RequestDetails {
    public String url;
    public byte[] postData;
    public long handle;
    public String[] headerKeys;
    public String[] headerValues;
};

class OxRequest extends Request<byte[]> implements Response.ErrorListener {
  public static abstract class Listener {
    public abstract void onResponse(OxRequest ox, byte[] response);
    public abstract void onFailure(OxRequest ox, VolleyError error);
  };
  @Override
  protected Response<byte[]> parseNetworkResponse(NetworkResponse response) {
    return Response.success(response.data, HttpHeaderParser.parseCacheHeaders(response));
  }
  @Override
  protected void deliverResponse(byte[] response) {
    listener.onResponse(this, response);
  }
  @Override
  public void deliverError(VolleyError error) {
    super.deliverError(error);
    listener.onFailure(this,error);
  }

  private Listener listener;
  private RequestDetails details;
  private Map<String, String> headers;
  public OxRequest(final RequestDetails d, OxRequest.Listener l) {
    super(d.postData == null?Request.Method.GET:Request.Method.POST,
          d.url, null);
    listener = l;
    details = d;
    headers = new HashMap<String, String>();
    for (int i = 0; i < d.headerKeys.length; ++i) {
      headers.put(d.headerKeys[i], d.headerValues[i]);
    }
  }
  @Override
  public Map<String,String> getHeaders() throws AuthFailureError {
    return headers;
  }
  @Override
  public byte[] getBody() throws AuthFailureError {
    return details.postData;
  }
  @Override
  public void onErrorResponse(VolleyError ve) {
    HttpRequest.nativeHttpRequestError(details.handle);
  }
  public long getHandle() { return details.handle; }
}

class HttpRequestHolder {

    private HttpRequest task;

    public HttpRequestHolder() {
    }
    public void run(final RequestDetails details) {
        OxygineActivity.instance.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                VolleyManager.addRequest(new OxRequest(details, new OxRequest.Listener() {
                  @Override
                  public void onResponse(OxRequest ox, byte[] r) {
                    HttpRequest.nativeHttpRequestGotHeader(ox.getHandle(), 200, r.length);
                    HttpRequest.nativeHttpRequestWrite(ox.getHandle(), r, r.length);
                    HttpRequest.nativeHttpRequestSuccess(ox.getHandle());
                  }
                  @Override
                  public void onFailure(OxRequest ox, VolleyError err) {
                    HttpRequest.nativeHttpRequestError(ox.getHandle());
                  }
                }));
            }
        });
    }
}

// usually, subclasses of AsyncTask are declared inside the activity class.
// that way, you can easily modify the UI thread from here
class HttpRequest {


    public HttpRequest() {
    }

    public static native void nativeHttpRequestSuccess(long handle);
    public static native void nativeHttpRequestError(long handle);
    public static native void nativeHttpRequestGotHeader(long handle, int code, int contentLength);
    public static native void nativeHttpRequestWrite(long handle, byte[] data, int size);
}


public class HttpRequests {

    static public void init() {

    }

    static public void release() {

    }

    static public HttpRequestHolder createRequest(String url, String[] headerKeys, String[] headerValues, byte[] post, long handle) {

        RequestDetails details = new RequestDetails();
        details.url = url;
        details.postData = post;
        details.handle = handle;
        details.headerKeys = headerKeys;
        details.headerValues = headerValues;

        HttpRequestHolder downloadTask = new HttpRequestHolder();
        downloadTask.run(details);

        return downloadTask;
    }
}
class VolleyManager {
  private static VolleyManager instance = null;
  private RequestQueue queue = null;
  private VolleyManager() { }
  public RequestQueue getQueue() {
    if (queue == null) {
      queue = Volley.newRequestQueue(OxygineActivity.instance.getApplicationContext());
    }
    return queue;
  }
  public static VolleyManager get() {
    if (instance == null) {
      instance = new VolleyManager();
    }
    return instance;
  }
  public static void addRequest(OxRequest r) {
    VolleyManager.get().getQueue().add(r);
  }
}
